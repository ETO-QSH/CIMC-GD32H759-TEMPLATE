import os
import numpy as np
import tensorflow as tf


def extract_weights_to_c(tflite_path='output/tictactoe_int8.tflite', out_dir='model'):
    """
    从 TFLite INT8 模型提取权重和量化参数。
    纯 shape+dtype 匹配，不依赖 tensor name。
    """
    os.makedirs(out_dir, exist_ok=True)

    # 定位 tflite 文件
    if not os.path.exists(tflite_path):
        candidates = [
            'output/tictactoe_int8.tflite',
            'output/tictactoe.tflite',
        ]
        found = None
        for c in candidates:
            if os.path.exists(c):
                found = c
                break
        if found is None:
            for root, _, files in os.walk('output'):
                for f in files:
                    if f.endswith('.tflite'):
                        found = os.path.join(root, f)
                        break
                if found:
                    break
        if found is None:
            raise FileNotFoundError(f"TFLite model not found: '{tflite_path}'")
        tflite_path = found

    interpreter = tf.lite.Interpreter(model_path=tflite_path)
    interpreter.allocate_tensors()

    tensor_details = interpreter.get_tensor_details()

    # 打印所有可读 tensor（调试用）
    print("      TFLite tensors:")
    readable = []
    for td in tensor_details:
        idx = td['index']
        name = td.get('name', '')
        shape = list(td.get('shape', []))
        try:
            arr = interpreter.get_tensor(idx)
            if arr is not None:
                readable.append({
                    'name': name,
                    'index': idx,
                    'shape': list(arr.shape),
                    'dtype': str(arr.dtype),
                    'size': arr.size,
                    'arr': arr,
                    'quant': td.get('quantization') or td.get('quantization_parameters') or ([], [])
                })
                print(
                    f"        [{idx:3d}] shape={str(list(arr.shape)):20s} dtype={str(arr.dtype):10s} size={arr.size:6d} {name}")
        except Exception:
            pass

    # 纯 shape+dtype 匹配定义
    targets = {
        'conv_w': {'shape': [32, 3, 3, 2], 'dtype': 'int8'},
        'conv_b': {'shape': [32], 'dtype': 'int32'},
        'fc1_w': {'shape': [64, 288], 'dtype': 'int8'},
        'fc1_b': {'shape': [64], 'dtype': 'int32'},
        'fc2_w': {'shape': [9, 64], 'dtype': 'int8'},
        'fc2_b': {'shape': [9], 'dtype': 'int32'},
    }

    found = {}
    for key, spec in targets.items():
        matches = []
        for info in readable:
            if info['shape'] == spec['shape'] and info['dtype'] == spec['dtype']:
                matches.append(info)

        if len(matches) == 0:
            raise RuntimeError(f"找不到 {key}: shape={spec['shape']} dtype={spec['dtype']}")
        if len(matches) > 1:
            names = [m['name'] for m in matches]
            raise RuntimeError(f"找到多个 {key}: {names}")

        found[key] = matches[0]
        print(f"        ✓ {key:6s} matched '{matches[0]['name']}'")

    # 提取量化参数
    def get_scale_zp(quant):
        """
        从 TFLite 量化参数中提取 scale 和 zero_point。
        处理 per-tensor 和 per-channel 两种情况。
        如果 scale 全为零或无效，fallback 到 1.0（避免除零）。
        """
        scale = 1.0
        zero_point = 0

        if isinstance(quant, (list, tuple)) and len(quant) >= 2:
            s = quant[0] if quant[0] is not None else 1.0
            z = quant[1] if quant[1] is not None else 0

            # 处理 scale
            if isinstance(s, (list, tuple, np.ndarray)) and len(s) > 0:
                s_arr = np.array(s, dtype=np.float32)
                non_zero = s_arr[s_arr != 0]
                scale = float(np.mean(non_zero)) if len(non_zero) > 0 else 1.0
            else:
                scale = float(s) if float(s) != 0.0 else 1.0

            # 处理 zero_point
            if isinstance(z, (list, tuple, np.ndarray)) and len(z) > 0:
                zero_point = int(np.mean(z))
            else:
                zero_point = int(z)

        elif isinstance(quant, dict):
            scales = quant.get('scales', None)
            zps = quant.get('zero_points', None)

            # 处理 scale
            if scales is not None:
                try:
                    s_arr = np.array(scales, dtype=np.float32)
                    if s_arr.size > 0:
                        non_zero = s_arr[s_arr != 0]
                        scale = float(np.mean(non_zero)) if len(non_zero) > 0 else 1.0
                except (TypeError, ValueError):
                    scale = 1.0

            # 处理 zero_point
            if zps is not None:
                try:
                    z_arr = np.array(zps)
                    if z_arr.size > 0:
                        zero_point = int(np.mean(z_arr))
                except (TypeError, ValueError):
                    zero_point = 0

        return scale, zero_point

    arrays = {}
    metas = {}
    for key in ['conv_w', 'conv_b', 'fc1_w', 'fc1_b', 'fc2_w', 'fc2_b']:
        info = found[key]
        arr = info['arr']
        arrays[key] = arr
        scale, zero_point = get_scale_zp(info['quant'])
        metas[key] = {
            'shape': info['shape'],
            'dtype': info['dtype'],
            'scale': scale,
            'zero_point': zero_point,
        }

    # 输入/输出 meta
    input_td = interpreter.get_input_details()[0]
    output_td = interpreter.get_output_details()[0]
    for key, td in [('input', input_td), ('output', output_td)]:
        q = td.get('quantization') or td.get('quantization_parameters') or ([], [])
        scale, zero_point = get_scale_zp(q)
        metas[key] = {
            'shape': list(td.get('shape', [])),
            'dtype': str(td.get('dtype')),
            'scale': scale,
            'zero_point': zero_point,
        }

    # 写入 model_weights.h
    weights_path = os.path.join(out_dir, 'model_weights.h')
    meta_path = os.path.join(out_dir, 'model_meta.h')

    with open(weights_path, 'w') as wf:
        wf.write('#ifndef MODEL_WEIGHTS_H\n')
        wf.write('#define MODEL_WEIGHTS_H\n\n')
        wf.write('#include <stdint.h>\n\n')
        for key in ['conv_w', 'conv_b', 'fc1_w', 'fc1_b', 'fc2_w', 'fc2_b']:
            arr = arrays[key]
            shape = list(arr.shape)
            dtype = str(arr.dtype)
            wf.write(f'// {key} shape {shape} dtype {dtype}\n')
            if np.issubdtype(arr.dtype, np.integer) and arr.dtype.itemsize == 1:
                ctype = 'int8_t'
            elif np.issubdtype(arr.dtype, np.integer) and arr.dtype.itemsize == 4:
                ctype = 'int32_t'
            else:
                ctype = 'int32_t'
            wf.write(f'const {ctype} {key}[] = {{\n  ')
            flat = arr.flatten()
            parts = [str(int(x)) for x in flat.tolist()]
            lines = []
            for i in range(0, len(parts), 12):
                lines.append(', '.join(parts[i:i + 12]))
            wf.write(',\n  '.join(lines))
            wf.write('\n};\n\n')
        wf.write('#endif // MODEL_WEIGHTS_H\n')

    # 写入 model_meta.h
    with open(meta_path, 'w') as mf:
        mf.write('#ifndef MODEL_META_H\n')
        mf.write('#define MODEL_META_H\n\n')
        for key in ['input', 'output', 'conv_w', 'conv_b', 'fc1_w', 'fc1_b', 'fc2_w', 'fc2_b']:
            meta = metas.get(key)
            if not meta:
                continue
            mf.write(f'// {key}\n')
            mf.write(f'#define {key.upper()}_SHAPE {{{", ".join(str(s) for s in meta["shape"])}}}\n')
            mf.write(f'#define {key.upper()}_DTYPE "{meta["dtype"]}"\n')
            mf.write(f'#define {key.upper()}_SCALE {meta["scale"]}f\n')
            mf.write(f'#define {key.upper()}_ZERO_POINT {meta["zero_point"]}\n\n')
        mf.write('#endif // MODEL_META_H\n')

    print(f'      写入 {weights_path} 和 {meta_path}')
    return weights_path, meta_path


def convert_to_c_array():
    with open("output/tictactoe_int8.tflite", "rb") as f:
        data = f.read()

    os.makedirs("model", exist_ok=True)
    with open("model/model_data.h", "w") as f:
        f.write("#ifndef MODEL_DATA_H\n#define MODEL_DATA_H\n\n")
        f.write(f"const unsigned int model_len = {len(data)};\n")
        f.write("__attribute__((aligned(8))) const unsigned char model_data[] = {\n  ")
        hex_str = ", ".join(f"0x{b:02x}" for b in data)
        lines = []
        parts = hex_str.split(", ")
        for i in range(0, len(parts), 12):
            lines.append(", ".join(parts[i:i + 12]))
        f.write(",\n  ".join(lines))
        f.write("\n};\n\n#endif\n")

    return data


if __name__ == "__main__":
    extract_weights_to_c()
    convert_to_c_array()

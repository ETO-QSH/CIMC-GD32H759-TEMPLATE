import os
import numpy as np
import tensorflow as tf


def extract_weights_to_c(tflite_path='output/tictactoe_int8.tflite', out_dir='model'):
    """
    从 TFLite INT8 模型提取权重和量化参数。
    支持 per-channel 量化，Conv 权重和偏置保存 32 个独立 scale。
    """
    os.makedirs(out_dir, exist_ok=True)

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

    print("      TFLite tensors:")
    readable = []
    for td in tensor_details:
        idx = td['index']
        name = td.get('name', '')
        shape = list(td.get('shape', []))
        try:
            arr = interpreter.get_tensor(idx)
            if arr is not None:
                # 同时保存 quantization 和 quantization_parameters
                quant = td.get('quantization')
                quant_params = td.get('quantization_parameters')
                readable.append({
                    'name': name,
                    'index': idx,
                    'shape': list(arr.shape),
                    'dtype': str(arr.dtype),
                    'size': arr.size,
                    'arr': arr,
                    'quant': quant,
                    'quant_params': quant_params,
                })
                print(
                    f"        [{idx:3d}] shape={str(list(arr.shape)):20s} dtype={str(arr.dtype):10s} size={arr.size:6d} {name}")
        except Exception:
            pass

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

    def get_scale_zp(quant_tuple, quant_params):
        """
        优先使用 quantization_parameters（支持 per-channel），
        fallback 到 quantization tuple。
        """
        # 先尝试 quantization_parameters（per-channel 数据在这里）
        if quant_params is not None:
            scales = quant_params.get('scales', None)
            zps = quant_params.get('zero_points', None)

            if scales is not None:
                try:
                    s_arr = np.array(scales, dtype=np.float32)
                    if s_arr.size > 0:
                        # 返回完整的 per-channel 数组，不取平均
                        return s_arr.tolist(), int(np.mean(zps)) if zps is not None else 0
                except (TypeError, ValueError):
                    pass

        # fallback 到 quantization tuple（per-tensor）
        if isinstance(quant_tuple, (list, tuple)) and len(quant_tuple) >= 2:
            s = quant_tuple[0] if quant_tuple[0] is not None else 1.0
            z = quant_tuple[1] if quant_tuple[1] is not None else 0
            scale = float(s) if float(s) != 0.0 else 1.0
            return [scale], int(z)

        return [1.0], 0

    arrays = {}
    metas = {}
    for key in ['conv_w', 'conv_b', 'fc1_w', 'fc1_b', 'fc2_w', 'fc2_b']:
        info = found[key]
        arr = info['arr']
        arrays[key] = arr

        scales, zero_point = get_scale_zp(info['quant'], info['quant_params'])
        metas[key] = {
            'shape': info['shape'],
            'dtype': info['dtype'],
            'scales': scales,  # 可能是 list（per-channel）或单元素 list
            'zero_point': zero_point,
        }

    # 输入/输出 meta
    input_td = interpreter.get_input_details()[0]
    output_td = interpreter.get_output_details()[0]
    for key, td in [('input', input_td), ('output', output_td)]:
        quant_params = td.get('quantization_parameters')
        quant = td.get('quantization')
        scales, zero_point = get_scale_zp(quant, quant_params)
        metas[key] = {
            'shape': list(td.get('shape', [])),
            'dtype': str(td.get('dtype')),
            'scales': scales,
            'zero_point': zero_point,
        }

    # 写入 model_weights.h
    weights_path = os.path.join(out_dir, 'model_weights.h')
    meta_path = os.path.join(out_dir, 'model_meta.h')

    with open(weights_path, 'w') as wf:
        wf.write('#ifndef MODEL_WEIGHTS_H\n')
        wf.write('#define MODEL_WEIGHTS_H\n\n')
        wf.write('#include <stdint.h>\n\n')

        # 权重数组
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

        # per-channel scale 数组（仅 Conv 层需要）
        wf.write('// conv_w per-channel scales (32 channels)\n')
        wf.write('const float conv_w_scale_per_channel[] = {\n  ')
        scales_str = [f'{s:.8f}f' for s in metas['conv_w']['scales']]
        lines = []
        for i in range(0, len(scales_str), 8):
            lines.append(', '.join(scales_str[i:i + 8]))
        wf.write(',\n  '.join(lines))
        wf.write('\n};\n\n')

        wf.write('// conv_b per-channel scales (32 channels)\n')
        wf.write('const float conv_b_scale_per_channel[] = {\n  ')
        scales_str = [f'{s:.8f}f' for s in metas['conv_b']['scales']]
        lines = []
        for i in range(0, len(scales_str), 8):
            lines.append(', '.join(scales_str[i:i + 8]))
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

            # per-tensor 的 scale 写单个值，per-channel 的写数量
            scales = meta['scales']
            if len(scales) == 1:
                mf.write(f'#define {key.upper()}_SCALE {scales[0]}f\n')
            else:
                mf.write(f'#define {key.upper()}_SCALE_COUNT {len(scales)}\n')
                # 同时写一个平均 scale 作为 fallback
                avg_scale = float(np.mean(scales))
                mf.write(f'#define {key.upper()}_SCALE {avg_scale}f\n')

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

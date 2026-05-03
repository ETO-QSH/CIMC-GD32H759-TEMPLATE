import tensorflow as tf
import numpy as np


def inspect_model(path):
    print(f"\n{'=' * 60}")
    print(f"Inspecting: {path}")
    print(f"{'=' * 60}")

    interpreter = tf.lite.Interpreter(model_path=path)
    interpreter.allocate_tensors()

    # 输入/输出
    for details in interpreter.get_input_details():
        print(f"\nInput: {details['name']}")
        print(f"  shape: {details['shape']}")
        print(f"  dtype: {details['dtype']}")
        q = details.get('quantization_parameters', {})
        print(f"  scale: {q.get('scales', [None])}")
        print(f"  zp:    {q.get('zero_points', [None])}")

    for details in interpreter.get_output_details():
        print(f"\nOutput: {details['name']}")
        print(f"  shape: {details['shape']}")
        print(f"  dtype: {details['dtype']}")
        q = details.get('quantization_parameters', {})
        print(f"  scale: {q.get('scales', [None])}")
        print(f"  zp:    {q.get('zero_points', [None])}")

    # 所有 tensor
    print(f"\n--- All Tensors ---")
    for td in interpreter.get_tensor_details():
        idx = td['index']
        name = td.get('name', '')
        shape = list(td.get('shape', []))
        try:
            arr = interpreter.get_tensor(idx)
            actual = list(arr.shape) if arr is not None else None
            dtype = str(arr.dtype) if arr is not None else 'N/A'
            size = arr.size if arr is not None else 0
        except Exception as e:
            actual = f"ERROR: {e}"
            dtype = 'N/A'
            size = 0

        # ✅ 修复：把 list 转成 str 再格式化
        shape_str = str(shape)
        actual_str = str(actual) if actual is not None else "None"
        print(f"  [{idx:3d}] {name:50s} shape={shape_str:20s} actual={actual_str:20s} dtype={dtype:10s} size={size}")

    # 快速推理测试
    print(f"\n--- Quick Inference Test ---")
    input_details = interpreter.get_input_details()[0]
    output_details = interpreter.get_output_details()[0]

    # 空棋盘测试
    test_input = np.zeros(input_details['shape'], dtype=input_details['dtype'])
    if input_details['dtype'] == np.int8:
        scale = input_details['quantization_parameters']['scales'][0]
        zp = input_details['quantization_parameters']['zero_points'][0]
        test_input = (test_input / scale + zp).astype(np.int8)

    interpreter.set_tensor(input_details['index'], test_input)
    interpreter.invoke()
    output = interpreter.get_tensor(output_details['index'])

    if output_details['dtype'] == np.int8:
        scale = output_details['quantization_parameters']['scales'][0]
        zp = output_details['quantization_parameters']['zero_points'][0]
        output = (output.astype(np.float32) - zp) * scale

    print(f"Empty board output: {output.flatten()}")
    print(f"Argmax: {output.argmax()}")


if __name__ == "__main__":
    inspect_model("output/tictactoe_fp32.tflite")
    inspect_model("output/tictactoe_int8.tflite")

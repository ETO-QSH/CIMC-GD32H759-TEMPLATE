import tensorflow as tf

from utils import representative_dataset


def quantize_model():
    # 1. 配置量化转换器
    converter = tf.lite.TFLiteConverter.from_saved_model("saved")
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    converter.representative_dataset = representative_dataset
    converter.target_spec.supported_ops = [
        tf.lite.OpsSet.TFLITE_BUILTINS_INT8
    ]
    # [修改] UINT8 → INT8：TFLite Micro 和 CMSIS-NN 对 INT8 的 kernel 优化更完善
    converter.inference_input_type = tf.int8
    converter.inference_output_type = tf.int8

    # 2. 执行量化
    tflite_quant_model = converter.convert()

    # 3. 保存模型
    with open("output/tictactoe_int8.tflite", "wb") as f:
        f.write(tflite_quant_model)

    # 4. 验证模型大小
    # [修改] 使用 with 语句，避免文件句柄未关闭
    with open("output/tictactoe_fp32.tflite", "rb") as f:
        fp32_size = len(f.read())
    int8_size = len(tflite_quant_model)
    print(f"      量化完成 | FP32: {fp32_size} bytes → INT8: {int8_size} bytes ({int8_size / fp32_size:.1%})")


if __name__ == "__main__":
    quantize_model()

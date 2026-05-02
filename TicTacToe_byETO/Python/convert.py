import torch
import numpy as np
import tensorflow as tf

from train import TicTacToeCNN


def convert_to_tflite():
    # 1. 加载 PyTorch 模型
    pt_model = TicTacToeCNN()
    pt_model.load_state_dict(torch.load("output/tictactoe_cnn.pth", map_location="cpu"))
    pt_model.eval()

    # 2. 提取 PyTorch 权重为 numpy
    # Conv2d: PyTorch 格式 [out_ch, in_ch, H, W] = [16, 2, 3, 3]
    pt_conv_w = pt_model.conv[0].weight.detach().cpu().numpy()
    pt_conv_b = pt_model.conv[0].bias.detach().cpu().numpy()

    # Linear: PyTorch 格式 [out, in] = [9, 144]
    pt_fc_w = pt_model.conv[3].weight.detach().cpu().numpy()
    pt_fc_b = pt_model.conv[3].bias.detach().cpu().numpy()

    # 3. 用 TF/Keras 构建等价模型 (原生 NHWC，与 MCU 端一致)
    tf_model = tf.keras.Sequential([
        tf.keras.layers.Conv2D(
            filters=16,
            kernel_size=3,
            padding="same",
            activation="relu",
            input_shape=(3, 3, 2),  # H=3, W=3, C=2
            name="conv2d"
        ),
        tf.keras.layers.Flatten(name="flatten"),
        tf.keras.layers.Dense(9, name="dense")
    ])

    # 4. 搬运权重 (关键：维度转置)
    # PyTorch Conv [16, 2, 3, 3] -> TF Conv [H, W, in, out] = [3, 3, 2, 16]
    tf_conv_w = np.transpose(pt_conv_w, (2, 3, 1, 0))
    tf_model.layers[0].set_weights([tf_conv_w, pt_conv_b])

    # PyTorch Linear [9, 144] -> TF Dense [in, out] = [144, 9]
    tf_fc_w = np.transpose(pt_fc_w, (1, 0))
    tf_model.layers[2].set_weights([tf_fc_w, pt_fc_b])

    # 5. 保存 SavedModel (供 quantize.py 使用)
    tf.saved_model.save(tf_model, "saved_model")
    print("✅ 中间模型 SavedModel 已生成")

    # 6. 转 TFLite (FP32)
    converter = tf.lite.TFLiteConverter.from_saved_model("saved_model")
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS]
    tflite_model = converter.convert()

    with open("output/tictactoe_fp32.tflite", "wb") as f:
        f.write(tflite_model)

    print("✅ FP32模型已生成: tictactoe_fp32.tflite")


if __name__ == "__main__":
    convert_to_tflite()

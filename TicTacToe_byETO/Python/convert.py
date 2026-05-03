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
    pt_conv_w = pt_model.conv[0].weight.detach().cpu().numpy()  # [32, 2, 3, 3]
    pt_conv_b = pt_model.conv[0].bias.detach().cpu().numpy()  # [32]

    pt_fc1_w = pt_model.conv[3].weight.detach().cpu().numpy()  # [64, 288]
    pt_fc1_b = pt_model.conv[3].bias.detach().cpu().numpy()  # [64]

    pt_fc2_w = pt_model.conv[5].weight.detach().cpu().numpy()  # [9, 64]
    pt_fc2_b = pt_model.conv[5].bias.detach().cpu().numpy()  # [9]

    # 3. 用 TF/Keras 构建等价模型 (原生 NHWC)
    tf_model = tf.keras.Sequential([
        tf.keras.layers.Conv2D(
            filters=32,
            kernel_size=3,
            padding="same",
            activation="relu",
            input_shape=(3, 3, 2),
            name="conv2d"
        ),
        tf.keras.layers.Flatten(name="flatten"),
        tf.keras.layers.Dense(64, activation="relu", name="dense1"),
        tf.keras.layers.Dense(9, name="dense")
    ])

    # 4. 搬运权重

    # Conv: PyTorch [32, 2, 3, 3] -> TF [H, W, in, out] = [3, 3, 2, 32]
    tf_conv_w = np.transpose(pt_conv_w, (2, 3, 1, 0))
    tf_model.layers[0].set_weights([tf_conv_w, pt_conv_b])

    # Dense1: PyTorch [64, 288] -> TF [in, out] = [288, 64]
    # 关键修复：PyTorch flatten 是 C-major，Keras flatten 是 HWC-major
    # 需要把权重从 [64, 32, 3, 3]  permute 到 [64, 3, 3, 32] 再 flatten
    pt_fc1_w_reshaped = pt_fc1_w.reshape(64, 32, 3, 3)  # [64, 32, 3, 3]
    pt_fc1_w_nhwc = pt_fc1_w_reshaped.transpose(0, 2, 3, 1)  # [64, 3, 3, 32]
    pt_fc1_w_nhwc_flat = pt_fc1_w_nhwc.reshape(64, 288)  # [64, 288]
    tf_fc1_w = np.transpose(pt_fc1_w_nhwc_flat, (1, 0))  # [288, 64]
    tf_model.layers[2].set_weights([tf_fc1_w, pt_fc1_b])

    # Dense2: PyTorch [9, 64] -> TF [in, out] = [64, 9]
    # FC2 不受 flatten 影响，正常转置即可
    tf_fc2_w = np.transpose(pt_fc2_w, (1, 0))
    tf_model.layers[3].set_weights([tf_fc2_w, pt_fc2_b])

    # 5. 保存 SavedModel
    tf.saved_model.save(tf_model, "saved")
    print("      中间模型 SavedModel 已生成")

    # 6. 转 TFLite (FP32)
    converter = tf.lite.TFLiteConverter.from_saved_model("saved")
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS]
    tflite_model = converter.convert()

    with open("output/tictactoe_fp32.tflite", "wb") as f:
        f.write(tflite_model)

    print("      FP32模型已生成: tictactoe_fp32.tflite")


if __name__ == "__main__":
    convert_to_tflite()

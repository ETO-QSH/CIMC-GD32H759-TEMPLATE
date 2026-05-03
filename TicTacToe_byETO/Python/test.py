import numpy as np
import torch
import tensorflow as tf
from train import TicTacToeCNN

# ============ 加载模型 ============

# 加载 PyTorch 模型
pt_model = TicTacToeCNN()
pt_model.load_state_dict(torch.load("output/tictactoe_cnn.pth", map_location="cpu"))
pt_model.eval()

# 加载 TFLite INT8 模型
interpreter_int8 = tf.lite.Interpreter(model_path="output/tictactoe_int8.tflite")
interpreter_int8.allocate_tensors()
input_details_int8 = interpreter_int8.get_input_details()[0]
output_details_int8 = interpreter_int8.get_output_details()[0]

# 加载 TFLite FP32 模型
interpreter_fp32 = tf.lite.Interpreter(model_path="output/tictactoe_fp32.tflite")
interpreter_fp32.allocate_tensors()
input_details_fp32 = interpreter_fp32.get_input_details()[0]
output_details_fp32 = interpreter_fp32.get_output_details()[0]


# ============ 推理函数 ============

def pt_predict(state_nchw):
    """state_nchw: (2,3,3), 当前玩家视角 [current, opponent]"""
    x = torch.tensor(state_nchw, dtype=torch.float32).unsqueeze(0)
    with torch.no_grad():
        out = pt_model(x)
    return out.numpy().flatten()


def tflite_int8_predict(state_nchw):
    """state_nchw: (2,3,3)"""
    # 转 NHWC
    nhwc = state_nchw.transpose(1, 2, 0)  # (3,3,2)
    nhwc = np.expand_dims(nhwc, 0).astype(np.float32)

    # 量化到 int8
    scale = input_details_int8['quantization_parameters']['scales'][0]
    zp = input_details_int8['quantization_parameters']['zero_points'][0]
    nhwc_int8 = (nhwc / scale + zp).astype(np.int8)

    interpreter_int8.set_tensor(input_details_int8['index'], nhwc_int8)
    interpreter_int8.invoke()
    out = interpreter_int8.get_tensor(output_details_int8['index'])

    # 反量化
    o_scale = output_details_int8['quantization_parameters']['scales'][0]
    o_zp = output_details_int8['quantization_parameters']['zero_points'][0]
    out_f = (out.astype(np.float32) - o_zp) * o_scale
    return out_f.flatten()


def tflite_fp32_predict(state_nchw):
    """state_nchw: (2,3,3)"""
    # 转 NHWC
    nhwc = state_nchw.transpose(1, 2, 0)  # (3,3,2)
    nhwc = np.expand_dims(nhwc, 0).astype(np.float32)

    interpreter_fp32.set_tensor(input_details_fp32['index'], nhwc)
    interpreter_fp32.invoke()
    out = interpreter_fp32.get_tensor(output_details_fp32['index'])
    return out.flatten()


# ============ 测试函数 ============

def compare_predictions(name, state_nchw):
    """对比三个模型的输出"""
    pt_out = pt_predict(state_nchw)
    fp32_out = tflite_fp32_predict(state_nchw)
    int8_out = tflite_int8_predict(state_nchw)

    print(f"\n=== {name} ===")
    print(f"PyTorch:   {pt_out}")
    print(f"TFLite FP32: {fp32_out}")
    print(f"TFLite INT8: {int8_out}")

    # 计算差异
    diff_fp32 = np.abs(pt_out - fp32_out)
    diff_int8 = np.abs(pt_out - int8_out)

    print(f"\n|PyTorch - FP32| max={diff_fp32.max():.6f}, mean={diff_fp32.mean():.6f}")
    print(f"|PyTorch - INT8| max={diff_int8.max():.6f}, mean={diff_int8.mean():.6f}")

    # 判断问题来源
    if diff_fp32.max() < 1e-4:
        print("\n✅ FP32 模型与 PyTorch 输出一致，问题出在 INT8 量化上")
    else:
        print("\n❌ FP32 模型与 PyTorch 输出不一致，问题出在 TFLite 转换/推理过程中")

    if diff_int8.max() < 1e-4:
        print("✅ INT8 模型与 PyTorch 输出一致")
    else:
        print(f"⚠️  INT8 量化引入误差: max={diff_int8.max():.6f}")


# ============ 测试用例 ============

# 测试局面 1：空棋盘，轮到 X
empty_board = np.zeros((2, 3, 3), dtype=np.float32)
compare_predictions("空棋盘（轮到 X）", empty_board)

# 测试局面 2：O 占中心，轮到 X
board_o_center = np.zeros((2, 3, 3), dtype=np.float32)
board_o_center[1, 1, 1] = 1.0
compare_predictions("O 占中心（轮到 X）", board_o_center)

# 测试局面 3：O 双杀威胁，轮到 X
board_threat = np.zeros((2, 3, 3), dtype=np.float32)
board_threat[0, 1, 1] = 1.0  # X 在中心
board_threat[1, 0, 0] = 1.0  # O 在 0
board_threat[1, 0, 1] = 1.0  # O 在 1
compare_predictions("O 双杀威胁（轮到 X，应该堵 2）", board_threat)

# 测试局面 4：X 即将获胜
board_x_win = np.zeros((2, 3, 3), dtype=np.float32)
board_x_win[0, 0, 0] = 1.0  # X 在 0
board_x_win[0, 0, 1] = 1.0  # X 在 1
board_x_win[1, 1, 1] = 1.0  # O 在 4
compare_predictions("X 即将获胜（X 在 0,1，O 在 4）", board_x_win)

# 测试局面 5：O 直线威胁
board_o_line = np.zeros((2, 3, 3), dtype=np.float32)
board_o_line[0, 0, 0] = 1.0  # X 在 0
board_o_line[1, 1, 1] = 1.0  # O 在 4
board_o_line[1, 1, 2] = 1.0  # O 在 5
compare_predictions("O 直线威胁（O 在 4,5，X 应该堵 3）", board_o_line)

import numpy as np
from itertools import product


def generate_all_valid_states():
    """生成所有合法的井字棋状态（跳过无效/终局状态）"""
    states = []
    # 枚举所有3x3棋盘可能性 (3^9=19683种)
    for board_flat in product([-1, 0, 1], repeat=9):
        board = np.array(board_flat).reshape(3, 3)

        # 过滤无效状态：X和O数量差>1，或一方已获胜但继续落子
        count_x = np.sum(board == 1)
        count_o = np.sum(board == -1)
        if abs(count_x - count_o) > 1:
            continue

        # 检查是否已分出胜负（跳过终局状态）
        if check_win(board, 1) or check_win(board, -1):
            continue

        # 转换为2通道输入格式: [O的位置, X的位置]
        o_channel = (board == -1).astype(np.float32)
        x_channel = (board == 1).astype(np.float32)
        states.append(np.stack([o_channel, x_channel]))

    return np.array(states)  # 形状: (N, 2, 3, 3)


def check_win(board, player):
    """检查player是否获胜"""
    # 行/列/对角线检查
    for i in range(3):
        if np.all(board[i, :] == player) or np.all(board[:, i] == player):
            return True
    if np.all(np.diag(board) == player) or np.all(np.diag(np.fliplr(board)) == player):
        return True
    return False


def representative_dataset():
    # 1. 加载数据
    x_np = np.load("output/calibration_data.npy")
    print(f"✅ 量化校准器：加载到 {len(x_np)} 个真实样本")

    # 2. 数据切片 (防止校准时间过长，通常几百个样本足够)
    num_calibration_steps = min(200, len(x_np))

    # 3. 格式转换与生成 (关键：NHWC 格式)
    for i in range(num_calibration_steps):
        # 提取单个样本 (2, 3, 3)
        sample = x_np[i]

        # 转换维度: (2, 3, 3) -> (3, 3, 2)
        sample_nhwc = sample.transpose(1, 2, 0)

        # 增加 Batch 维度: (1, 3, 3, 2)
        sample_batched = np.expand_dims(sample_nhwc, axis=0).astype(np.float32)

        # [新增] 将 [0,1] 映射到 [0,255]，使量化 scale≈1.0，方便 MCU 端直接传整型
        sample_batched *= 255.0

        yield [sample_batched]  # 返回列表

    print(f"✅ 量化校准器：已提供 {num_calibration_steps} 个样本完成校准")


def convert_to_c_array():
    with open("output/tictactoe_int8.tflite", "rb") as f:
        data = f.read()

    with open("output/model_data.h", "w") as f:
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

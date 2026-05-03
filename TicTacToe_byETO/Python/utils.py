import numpy as np
from itertools import product


def generate_all_valid_states():
    """生成所有合法的井字棋状态（O 先手规则）"""
    states = []
    for board_flat in product([-1, 0, 1], repeat=9):
        board = np.array(board_flat).reshape(3, 3)

        count_x = np.sum(board == 1)
        count_o = np.sum(board == -1)

        # ✅ 修正：O 先手规则下，O 数量必须 >= X 数量，且最多多 1
        if count_o < count_x or count_o > count_x + 1:
            continue

        # 跳过终局状态（已分出胜负，无需下子）
        if check_win(board, 1) or check_win(board, -1):
            continue

        # 固定通道：[O的位置, X的位置]
        o_channel = (board == -1).astype(np.float32)
        x_channel = (board == 1).astype(np.float32)
        states.append(np.stack([o_channel, x_channel]))

    return np.array(states)


def check_win(board, player):
    for i in range(3):
        if np.all(board[i, :] == player) or np.all(board[:, i] == player):
            return True
    if np.all(np.diag(board) == player) or np.all(np.diag(np.fliplr(board)) == player):
        return True
    return False


def representative_dataset():
    x_np = np.load("output/calibration_data.npy")
    print(f"      量化校准器：加载到 {len(x_np)} 个真实样本")
    num_calibration_steps = min(200, len(x_np))

    for i in range(num_calibration_steps):
        sample = x_np[i]                          # (2, 3, 3)
        sample_nhwc = sample.transpose(1, 2, 0)    # (3, 3, 2)
        sample_batched = np.expand_dims(sample_nhwc, axis=0).astype(np.float32)
        yield [sample_batched]

    print(f"      量化校准器：已提供 {num_calibration_steps} 个样本完成校准")

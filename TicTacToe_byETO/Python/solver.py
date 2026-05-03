import numpy as np


def get_winner(board):
    for i in range(3):
        if board[i, 0] == board[i, 1] == board[i, 2] != 0:
            return board[i, 0]
        if board[0, i] == board[1, i] == board[2, i] != 0:
            return board[0, i]
    if board[0, 0] == board[1, 1] == board[2, 2] != 0:
        return board[0, 0]
    if board[0, 2] == board[1, 1] == board[2, 0] != 0:
        return board[0, 2]
    return 0


def get_valid_moves(board):
    return [i for i in range(9) if board.flat[i] == 0]


def minimax(board, player):
    winner = get_winner(board)
    if winner == 1:          # X 赢
        return -1, 10
    elif winner == -1:       # O 赢
        return -1, -10
    elif len(get_valid_moves(board)) == 0:
        return -1, 0

    best_move = -1
    if player == 1:          # MAX (X)
        best_value = -np.inf
        for move in get_valid_moves(board):
            new_board = board.copy()
            new_board.flat[move] = player
            _, value = minimax(new_board, -1)
            if value > best_value:
                best_value = value
                best_move = move
        return best_move, best_value
    else:                    # MIN (O)
        best_value = np.inf
        for move in get_valid_moves(board):
            new_board = board.copy()
            new_board.flat[move] = player
            _, value = minimax(new_board, 1)
            if value < best_value:
                best_value = value
                best_move = move
        return best_move, best_value


def solve_state(board):
    count_x = np.sum(board == 1)
    count_o = np.sum(board == -1)
    # O 先手：O 和 X 数量相等 → 轮到 O；否则 → 轮到 X
    if count_o == count_x:
        current_player = -1
    else:
        current_player = 1
    move, _ = minimax(board, current_player)
    return move if move != -1 else 0


def generate_dataset_from_minimax(states_2channel):
    """
    核心修改：输出"当前玩家视角"的数据
    Channel 0 = 当前行动方, Channel 1 = 对手
    """
    x, y = [], []

    for state in states_2channel:
        # state[0] = O, state[1] = X（固定视角）
        board = np.zeros((3, 3))
        board[state[0] == 1] = -1  # O
        board[state[1] == 1] = 1   # X

        count_o = np.sum(board == -1)
        count_x = np.sum(board == 1)

        # 判断当前轮到谁
        if count_o == count_x:
            current_player = -1   # 轮到 O
        else:
            current_player = 1    # 轮到 X

        best_move = solve_state(board)

        # ✅ 关键：转换为"当前玩家视角"
        # 模型永远学习"Channel 0 的玩家的最优策略"
        if current_player == -1:  # O 是当前玩家
            current_ch = state[0]   # O
            opponent_ch = state[1]  # X
        else:                     # X 是当前玩家
            current_ch = state[1]   # X
            opponent_ch = state[0]  # O

        x.append(np.stack([current_ch, opponent_ch]))
        y.append(best_move)

    return np.array(x), np.array(y)

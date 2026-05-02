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
    if winner == 1:
        return -1, 10
    elif winner == -1:
        return -1, -10
    elif len(get_valid_moves(board)) == 0:
        return -1, 0

    best_move = -1
    if player == 1:  # MAX (X)
        best_value = -np.inf
        for move in get_valid_moves(board):
            new_board = board.copy()
            new_board.flat[move] = player
            _, value = minimax(new_board, -1)
            if value > best_value:
                best_value = value
                best_move = move
        return best_move, best_value
    else:  # MIN (O)
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
    # [修改] 显式统计 X/O 数量，逻辑更清晰，避免 np.sum(board)==0 的歧义
    count_x = np.sum(board == 1)
    count_o = np.sum(board == -1)
    current_player = 1 if count_x == count_o else -1
    move, _ = minimax(board, current_player)
    return move if move != -1 else 0


def generate_dataset_from_minimax(states_2channel):
    """
    核心函数：输入2通道状态，输出 (X, y) 数据集
    """
    print(f"正在使用 Minimax 计算 {len(states_2channel)} 个状态的最佳落子...")
    x, y = [], []

    for state in states_2channel:
        # 转换格式: (2,3,3) -> (3,3)
        board = np.zeros((3, 3))
        board[state[0] == 1] = -1  # O
        board[state[1] == 1] = 1  # X

        best_move = solve_state(board)

        # 存储数据
        x.append(state)  # 保持2通道格式
        y.append(best_move)

    return np.array(x), np.array(y)

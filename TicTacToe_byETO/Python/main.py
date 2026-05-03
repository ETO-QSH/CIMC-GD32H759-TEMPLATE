import os
import time

import torch
import numpy as np
import torch.nn as nn
from torch.utils.data import DataLoader, TensorDataset

import convert
import quantize
from train import TicTacToeCNN
from utils import generate_all_valid_states
from solver import generate_dataset_from_minimax
from model import extract_weights_to_c, convert_to_c_array

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'


def main():
    print("🚀 启动井字棋AI训练工程...")
    start_time = time.time()
    os.mkdir('output')

    print("[1/6] 生成合法棋盘状态...")
    raw_states = generate_all_valid_states()
    print(f"      生成 {len(raw_states)} 个合法状态")

    print("[2/6] 运行 Minimax 算法生成最优标签...")
    x_np, y_np = generate_dataset_from_minimax(raw_states)

    print("      正在保存量化校准数据")
    np.save("output/calibration_data.npy", x_np)
    print(f"      已保存 {len(x_np)} 个样本")

    x_tensor = torch.tensor(x_np, dtype=torch.float32)
    y_tensor = torch.tensor(y_np, dtype=torch.long)

    dataset = TensorDataset(x_tensor, y_tensor)
    dataloader = DataLoader(dataset, batch_size=64, shuffle=True)

    print("[3/6] 开始训练 CNN 模型...")
    model = TicTacToeCNN()
    criterion = nn.CrossEntropyLoss()

    # 学习率加大，加衰减
    optimizer = torch.optim.Adam(model.parameters(), lr=0.005)
    scheduler = torch.optim.lr_scheduler.StepLR(optimizer, step_size=200, gamma=0.2)

    epochs = 300
    for epoch in range(epochs):
        model.train()
        total_loss = 0
        for batch_x, batch_y in dataloader:
            optimizer.zero_grad()
            outputs = model(batch_x)
            loss = criterion(outputs, batch_y)
            loss.backward()
            optimizer.step()
            total_loss += loss.item()

        scheduler.step()  # 每 epoch 衰减

        if (epoch + 1) % 50 == 0:
            avg_loss = total_loss / len(dataloader)
            model.eval()
            correct = 0
            with torch.no_grad():
                for i in range(len(x_tensor)):
                    pred = model(x_tensor[i:i + 1]).argmax(dim=1).item()
                    if pred == y_tensor[i].item():
                        correct += 1
            acc = correct / len(x_tensor)
            print(f"      Epoch {epoch + 1:3d}/{epochs}, Loss: {avg_loss:.4f}, Acc: {acc:.2%}")

    print("[4/6] 保存训练好的 PyTorch 模型...")
    torch.save(model.state_dict(), "output/tictactoe_cnn.pth")

    print("[5/6] 调用 convert.py 进行 TFLite 转换...")
    try:
        convert.convert_to_tflite()
    except Exception as e:
        print(f"      转换失败: {e}")

    print("[6/6] 调用 quantize.py 进行 INT8 量化...")
    try:
        quantize.quantize_model()
    except Exception as e:
        print(f"      量化失败: {e}")

    print("[7/7] 生成 C 头文件...")
    try:
        weights_path, meta_path = extract_weights_to_c()
        print(f"      已生成 {weights_path} 和 {meta_path}")
    except Exception as e:
        print(f"      生成失败: {e}")

    data = convert_to_c_array()
    print(f"      已生成 model_data.h  {len(data)} bytes")

    total_duration = time.time() - start_time
    print(f"\n🎉 全流程任务完成！总耗时: {total_duration:.2f} 秒")


if __name__ == "__main__":
    main()

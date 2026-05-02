import os
import time

import torch
import numpy as np
import torch.nn as nn
from torch.utils.data import DataLoader, TensorDataset

import convert
import quantize
from train import TicTacToeCNN
from solver import generate_dataset_from_minimax
from utils import generate_all_valid_states, convert_to_c_array

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'


def main():
    print("🚀 启动井字棋AI训练工程...")
    start_time = time.time()

    # --- 阶段 1：数据准备 ---
    print("[1/7] 📊 生成合法棋盘状态...")
    raw_states = generate_all_valid_states()
    print(f"   ✅ 生成 {len(raw_states)} 个合法状态")

    # --- 阶段 2：标签生成 (核心逻辑) ---
    print("[2/7] 🧠 运行 Minimax 算法生成最优标签...")
    x_np, y_np = generate_dataset_from_minimax(raw_states)

    # --- 阶段 2：保存校准数据 ---
    print("💾 正在保存量化校准数据 (calibration_data.npy) ...")
    np.save("output/calibration_data.npy", x_np)
    print(f" ✅ 已保存 {len(x_np)} 个样本用于后续量化校准")

    # 转为 Tensor
    x_tensor = torch.tensor(x_np, dtype=torch.float32)
    y_tensor = torch.tensor(y_np, dtype=torch.long)

    # 创建 Dataloader
    dataset = TensorDataset(x_tensor, y_tensor)
    dataloader = DataLoader(dataset, batch_size=64, shuffle=True)
    print("   ✅ 最优标签生成完毕，数据已加载")

    # --- 阶段 3：模型训练 ---
    print("[3/7] ⚙️  开始训练 CNN 模型...")
    model = TicTacToeCNN()
    criterion = nn.CrossEntropyLoss()
    optimizer = torch.optim.Adam(model.parameters(), lr=0.001)

    epochs = 100
    for epoch in range(epochs):
        total_loss = 0
        for batch_x, batch_y in dataloader:
            optimizer.zero_grad()
            outputs = model(batch_x)
            loss = criterion(outputs, batch_y)
            loss.backward()
            optimizer.step()
            total_loss += loss.item()

        if (epoch + 1) % 20 == 0:
            avg_loss = total_loss / len(dataloader)
            print(f"   Epoch {epoch + 1}/{epochs}, Avg Loss: {avg_loss:.4f}")

    # --- 阶段 4：保存 PyTorch 模型 ---
    print("[4/7] 💾 保存训练好的 PyTorch 模型...")
    torch.save(model.state_dict(), "output/tictactoe_cnn.pth")
    print("   ✅ 模型已保存")

    # --- 阶段 5：模型转换 (调用 convert.py) ---
    print("[5/7] 🐍 调用 convert.py 进行 ONNX & TFLite 转换...")
    try:
        convert.convert_to_tflite()
        print("   ✅ 转换流程执行完毕")
    except Exception as e:
        print(f"   ❌ 转换失败: {e}")

    # --- 阶段 6：模型量化 (调用 quantize.py) ---
    print("[6/7] ⚡ 调用 quantize.py 进行 INT8 量化...")
    try:
        quantize.quantize_model()
        print("   ✅ 量化流程执行完毕")
    except Exception as e:
        print(f"   ❌ 量化失败: {e}")

    # --- 阶段 7：生成 C 头文件 ---
    print("[7/7] ⚙️ 生成 C 头文件...")
    data = convert_to_c_array()
    print(f"   ✅ 已生成 model_data.h  {len(data)} bytes")

    # 结束计时也移到了循环外，统计的是全流程真实耗时
    total_duration = time.time() - start_time
    print(f"\n🎉 全流程任务完成！")
    print(f"⏱️ 总耗时: {total_duration:.2f} 秒")


if __name__ == "__main__":
    main()

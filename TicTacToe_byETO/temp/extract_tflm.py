#!/usr/bin/env python3
"""
精简 TFLM 提取脚本：只复制 GD32H759 井字棋 CNN 所需的最小文件集
需求：Conv2D, ReLU, Dense, FlatBuffer 解析，以及核心的内存管理与解释器
"""
import os
import shutil
import subprocess
import sys
from pathlib import Path

# 配置
TFLM_REPO = "https://github.com/tensorflow/tflite-micro.git"
CLONE_DIR = "./_tflm_repo"  # 临时克隆目录
OUTPUT_DIR = "../Middleware/TFLiteMicro"  # 目标输出目录

# 定义最小必需的目录结构
# 相对于 tflite-micro 仓库根目录
MINIMAL_DIRS = [
    # 核心微解释器与内存管理
    "tensorflow/lite/micro",
    # 算子解析 API
    "tensorflow/lite/core/api",
    # C API 封装
    "tensorflow/lite/c/common.c",
    "tensorflow/lite/c/common.h",
    # 参考算子实现（只复制需要的）
    "tensorflow/lite/kernels/internal",
    # FlatBuffer schema
    "tensorflow/lite/schema",
]

def run_cmd(cmd, cwd=None):
    """执行命令并返回返回码"""
    print(f"[CMD] {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"[ERR] {result.stderr}")
        return False
    print(f"[OK]  {result.stdout[:200]}" if result.stdout else "[OK]")
    return True

def clone_tflm():
    """克隆 TFLM 仓库（仅历史深度 1，节省时间）"""
    print("\n=== 第一步：克隆 TFLM 仓库（深度1加快速度）===")
    if os.path.exists(CLONE_DIR):
        print(f"[INFO] {CLONE_DIR} 已存在，跳过克隆")
        return True
    
    if not run_cmd(["git", "clone", "--depth", "1", TFLM_REPO, CLONE_DIR]):
        return False
    print(f"[✓] 克隆完成，路径: {CLONE_DIR}")
    return True

def copy_minimal_files():
    """只复制必需的文件，过滤掉测试/示例/非.c/.h文件"""
    print("\n=== 第二步：复制最小文件集合 ===")
    
    if not os.path.exists(CLONE_DIR):
        print(f"[ERR] {CLONE_DIR} 不存在，请先克隆")
        return False
    
    # 创建输出目录
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    
    # 定义要复制的路径前缀（支持通配符概念）
    essential_patterns = [
        "tensorflow/lite/micro/*.cc",
        "tensorflow/lite/micro/*.c",
        "tensorflow/lite/micro/*.h",
        "tensorflow/lite/micro/kernels/*.cc",
        "tensorflow/lite/micro/kernels/*.c",
        "tensorflow/lite/micro/kernels/*.h",
        "tensorflow/lite/micro/memory_planner/*.cc",
        "tensorflow/lite/micro/memory_planner/*.h",
        "tensorflow/lite/core/api/*.cc",
        "tensorflow/lite/core/api/*.h",
        "tensorflow/lite/c/common.c",
        "tensorflow/lite/c/common.h",
        "tensorflow/lite/kernels/internal/common.h",
        "tensorflow/lite/kernels/internal/types.h",
        "tensorflow/lite/kernels/internal/quantization_util.h",
        "tensorflow/lite/kernels/internal/reference/conv.h",
        "tensorflow/lite/kernels/internal/reference/fully_connected.h",
        "tensorflow/lite/kernels/internal/reference/activations.h",
        "tensorflow/lite/schema/*.h",
    ]
    
    copied_count = 0
    for pattern in essential_patterns:
        src_dir = os.path.join(CLONE_DIR, os.path.dirname(pattern))
        if not os.path.exists(src_dir):
            print(f"[SKIP] {src_dir} 不存在")
            continue
        
        for root, dirs, files in os.walk(src_dir):
            # 跳过测试目录
            dirs[:] = [d for d in dirs if "test" not in d and "testing" not in d and "example" not in d]
            
            for file in files:
                # 只复制源文件和头文件
                if file.endswith((".c", ".h", ".cc", ".hpp")):
                    src_file = os.path.join(root, file)
                    # 保留相对目录结构
                    rel_path = os.path.relpath(src_file, CLONE_DIR)
                    dst_file = os.path.join(OUTPUT_DIR, rel_path)
                    
                    os.makedirs(os.path.dirname(dst_file), exist_ok=True)
                    shutil.copy2(src_file, dst_file)
                    copied_count += 1
                    print(f"[+] {rel_path}")
    
    print(f"\n[✓] 共复制 {copied_count} 个文件到 {OUTPUT_DIR}")
    return True

def generate_keil_file_list():
    """生成 Keil 可用的文件列表（仅 .c 文件）"""
    print("\n=== 第三步：生成 Keil 编译文件清单 ===")
    
    keil_files = []
    for root, dirs, files in os.walk(OUTPUT_DIR):
        dirs[:] = [d for d in dirs if d != "__pycache__"]
        
        for file in files:
            if file.endswith((".c", ".cc")):
                full_path = os.path.join(root, file)
                # 转换为相对路径（从 project 目录看）
                rel_path = os.path.relpath(full_path, "..")
                keil_files.append(rel_path)
    
    # 保存清单
    with open("keil_tflm_files.txt", "w") as f:
        f.write("# TFLM 核心文件清单（Arm-ADS 编译用）\n")
        f.write("# 放入 Keil 的 Middleware_TFLM 分组\n\n")
        for file in sorted(keil_files):
            f.write(f"{file}\n")
    
    print(f"[✓] 文件清单已保存：keil_tflm_files.txt ({len(keil_files)} 个 .c/.cc 文件)")
    return True

def main():
    print("=" * 70)
    print("TFLite Micro 精简提取工具（为 GD32H759 井字棋 CNN 优化）")
    print("=" * 70)
    
    if not clone_tflm():
        print("\n[ERR] 克隆失败，检查网络和 git")
        return 1
    
    if not copy_minimal_files():
        print("\n[ERR] 文件复制失败")
        return 1
    
    if not generate_keil_file_list():
        print("\n[ERR] 文件清单生成失败")
        return 1
    
    print("\n" + "=" * 70)
    print("✓ 完成！下一步：")
    print("  1. 检查 Middleware/TFLiteMicro 目录结构")
    print("  2. 用 keil_tflm_files.txt 中的文件更新 Keil 项目的 Middleware_TFLM 分组")
    print("  3. 调整编译优化选项（推荐 O2，并启用 LTO）")
    print("=" * 70)
    return 0

if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""
生成 GD32H759 专用的精简 TFLM 编译清单
只保留：
  - 通用参考实现
  - 核心解释器 & 内存管理
  - 不包括平台特定优化（arc_mli, ceva, cmsis_nn, xtensa, ethos_u）
  - 不包括测试文件
"""

import os
import re

KEIL_INPUT = "keil_tflm_files.txt"
KEIL_OUTPUT = "keil_tflm_files_minimal.txt"

# 过滤规则：排除这些模式的文件
EXCLUDE_PATTERNS = [
    r"_test\.cc",          # 测试文件
    r"_test\.c",           # 测试文件
    r"/arc_mli/",          # ARC 优化
    r"/ceva/",             # CEVA 优化
    r"/cmsis_nn/",         # CMSIS-NN 优化（这个其实很有用，但对 GD32 不是必需）
    r"/ethos_u/",          # Ethos-U 优化
    r"/xtensa/",           # Xtensa 优化
    r"/hexagon/",          # Hexagon 优化
    r"/arc_custom/",       # ARC 定制
    r"/arc_emsdp/",        # ARC EMSDP
    r"/bluepill/",         # Blue Pill
    r"/chre/",             # CHRE
    r"/cortex_m_corstone", # Corstone 特定
    r"/ceva/",             # CEVA 特定
    r"/hexagon/",          # Hexagon 特定
    r"benchmarks/",        # 基准测试
    r"compression/",       # 压缩相关
    r"flex",               # FlexBuffer 特定
]

def should_include(line):
    """判断是否应该包含这一行"""
    line_lower = line.lower()
    
    for pattern in EXCLUDE_PATTERNS:
        if re.search(pattern, line_lower):
            return False
    
    # 只保留 .c 和 .cc 文件
    if not (line.endswith(".c") or line.endswith(".cc")):
        return False
    
    return True

def main():
    print(f"[*] Reading: {KEIL_INPUT}")
    with open(KEIL_INPUT, 'r', encoding='utf-8', errors='ignore') as f:
        lines = f.readlines()
    
    # 过滤
    filtered = []
    for line in lines:
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        if should_include(line):
            filtered.append(line)
    
    # 排序
    filtered.sort()
    
    # Save
    print(f"[*] Output to: {KEIL_OUTPUT}")
    with open(KEIL_OUTPUT, 'w', encoding='utf-8') as f:
        f.write("# GD32H759 TicTacToe CNN - TFLM Minimal Compilation List\n")
        f.write(f"# Total {len(filtered)} files\n")
        f.write("# Excluded: tests, platform-specific ops, benchmarks\n\n")
        for file in filtered:
            f.write(f"{file}\n")
    
    print(f"[OK] Done! {len(filtered)} source files (from original 600+)")
    print(f"\nNext steps:")
    print(f"  1. Open Keil IDE")
    print(f"  2. Right-click Middleware_TFLM group → Add files")
    print(f"  3. Copy file paths from {KEIL_OUTPUT}")
    print(f"  4. Or batch-add to .uvprojx via XML script")

if __name__ == "__main__":
    main()

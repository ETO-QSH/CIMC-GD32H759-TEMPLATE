#!/usr/bin/env python3
"""
自动把 TFLM 源文件添加到 Keil .uvprojx 项目文件（XML）
"""
import xml.etree.ElementTree as ET
from pathlib import Path

def add_files_to_keil_project():
    proj_file = "../project/tripleT.uvprojx"
    files_list = "keil_tflm_files_minimal.txt"
    
    # 读取源文件清单
    files = []
    with open(files_list, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith('#'):
                files.append(line)
    
    print(f"[*] Found {len(files)} files to add")
    
    # 解析 Keil 项目
    ET.register_namespace('', 'http://www.w3.org/2001/XMLSchema-instance')
    tree = ET.parse(proj_file)
    root = tree.getroot()
    
    # 找到 Middleware_TFLM 分组
    middleware_group = None
    for group in root.findall('.//Group'):
        group_name = group.find('GroupName')
        if group_name is not None and group_name.text == 'Middleware_TFLM':
            middleware_group = group
            break
    
    if middleware_group is None:
        print("[ERR] Middleware_TFLM group not found!")
        return False
    
    # 获取或创建 Files 元素
    files_elem = middleware_group.find('Files')
    if files_elem is None:
        files_elem = ET.SubElement(middleware_group, 'Files')
    
    # 添加文件
    added = 0
    for file_path in files:
        file_elem = ET.SubElement(files_elem, 'File')
        
        # FileName: 取最后一个路径部分
        file_name = Path(file_path).name
        fn = ET.SubElement(file_elem, 'FileName')
        fn.text = file_name
        
        # FileType: 1 for .c, 1 for .cc (both are code)
        ft = ET.SubElement(file_elem, 'FileType')
        ft.text = '1'
        
        # FilePath: 相对路径
        fp = ET.SubElement(file_elem, 'FilePath')
        fp.text = f"..\{file_path.replace('/', '\\')}"
        
        added += 1
        if added % 50 == 0:
            print(f"  [+] Added {added}/{len(files)} files...")
    
    print(f"  [+] Total added: {added} files")
    
    # 保存修改
    tree.write(proj_file, encoding='UTF-8', xml_declaration=True)
    print(f"[OK] Project file updated: {proj_file}")
    
    return True

if __name__ == "__main__":
    add_files_to_keil_project()

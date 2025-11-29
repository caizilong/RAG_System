#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
完整演示程序 - 展示数据处理和搜索功能

功能:
1. 将车辆手册文本转换为向量数据库
2. 显示生成的文件结构
3. 可选启动交互式搜索界面

用法: python run_demo.py
"""

import os
import sys
import time
import sentence_transformers
import numpy as np
import sklearn

from vehicle_data_processor import VehicleDataProcessor
from vehicle_vector_search import VehicleVectorSearch


def print_demo_banner():
    """打印演示程序标题"""
    print("=" * 80)
    print("车载文本向量化系统演示")
    print("基于 text2vec-base-chinese 模型")
    print("=" * 80)
    print()


def demo_data_processing(model_path):
    """
    数据处理演示 - 将文本转换为向量数据库

    流程:
    1. 检查输入文件是否存在
    2. 创建数据处理器
    3. 执行完整的数据处理流程
    4. 计时并显示结果

    Args:
        model_path: 文本向量化模型路径

    Returns:
        True - 处理成功, False - 处理失败
    """
    print("\n数据处理和向量化")
    print("-" * 50)

    # 检查输入文件
    input_file = "vehicle_manual_data.txt"
    if not os.path.exists(input_file):
        print(f"输入文件不存在: {input_file}")
        return False

    try:
        # 创建数据处理器
        processor = VehicleDataProcessor(os.path.normpath(model_path))

        # 执行完整的数据处理流程
        start_time = time.time()
        processor.process_vehicle_data(input_file)
        end_time = time.time()

        print(f"数据处理完成!耗时: {end_time - start_time:.2f} 秒")
        return True

    except Exception as e:
        print(f"数据处理失败: {e}")
        return False


def demo_interactive_search(model_path):
    """
    交互式搜索演示 - 启动命令行搜索界面

    Args:
        model_path: 文本向量化模型路径

    Returns:
        True - 成功, False - 失败
    """
    print("\n交互式搜索演示")
    print("-" * 50)

    try:
        from interactive_search import interactive_search
        print("启动交互式搜索界面...")
        print("在交互界面中,您可以尝试以下查询:")
        print("  - 发动机故障")
        print("  - 制动系统")
        print("  - 自动泊车")
        print("\n输入 'quit' 退出演示")

        # 启动交互式搜索
        interactive_search(model_path)
        return True

    except Exception as e:
        print(f"交互式搜索失败: {e}")
        return False


def show_file_structure():
    """
    显示生成的文件结构

    遍历vector_db目录,显示所有生成的文件及其大小
    """
    print("\n生成的文件结构:")
    print("-" * 50)

    vector_db_dir = "vector_db"
    if os.path.exists(vector_db_dir):
        # 遍历目录树
        for root, dirs, files in os.walk(vector_db_dir):
            level = root.replace(vector_db_dir, '').count(os.sep)
            indent = ' ' * 2 * level
            print(f"{indent}{os.path.basename(root)}/")
            subindent = ' ' * 2 * (level + 1)
            # 显示文件及大小
            for file in files:
                file_path = os.path.join(root, file)
                file_size = os.path.getsize(file_path)
                print(f"{subindent}{file} ({file_size} bytes)")
    else:
        print("vector_db 目录不存在")


def main():
    """
    主函数 - 执行完整的演示流程

    流程:
    1. 打印标题
    2. 执行数据处理(生成向量数据库)
    3. 显示生成的文件结构
    4. 询问是否启动交互式搜索

    Returns:
        0 - 成功, 1 - 失败
    """
    # 步骤1: 打印标题
    print_demo_banner()

    # 步骤2: 获取模型路径
    base_dir = os.path.dirname(os.path.abspath(__file__))
    model_path = os.path.join(base_dir, "..", "models")

    # 步骤3: 执行数据处理
    if not demo_data_processing(model_path):
        print("数据处理失败,无法继续演示")
        return 1

    # 步骤4: 显示生成的文件
    show_file_structure()

    # 步骤5: 询问是否启动交互式搜索
    print("\n是否启动交互式搜索界面? (y/n): ", end="")
    try:
        user_input = input().strip().lower()
        if user_input in ['y', 'yes', '是']:
            demo_interactive_search(model_path)
    except KeyboardInterrupt:
        print("\n演示结束")

    print("\n演示完成!")

    return 0


if __name__ == "__main__":
    exit_code = main()
    sys.exit(exit_code)

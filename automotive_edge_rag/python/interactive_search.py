#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
交互式搜索工具 - 命令行交互界面

功能: 提供一个简单的命令行界面,让用户可以交互式地搜索车辆手册内容
用法: python interactive_search.py
"""

import os
import sys
from vehicle_vector_search import VehicleVectorSearch


def print_sections(searcher):
    """
    打印所有章节列表

    Args:
        searcher: VehicleVectorSearch搜索器实例
    """
    stats = searcher.get_statistics()
    print(f"\n共有 {stats['total_sections']} 个章节:")
    for i, section in enumerate(stats['sections'], 1):
        print(f"  {i}. {section}")


def interactive_search(model_path):
    """
    交互式搜索主函数

    功能:
    1. 加载向量数据库
    2. 显示数据库统计信息
    3. 进入循环,接收用户输入并返回搜索结果

    Args:
        model_path: 文本向量化模型路径

    Returns:
        0 - 成功, 1 - 失败
    """
    try:
        # 加载向量搜索器
        print("正在加载向量数据库...")
        searcher = VehicleVectorSearch(model_path)
        print("向量数据库加载成功!")

        # 显示数据库统计信息
        stats = searcher.get_statistics()
        print(f"\n数据库信息:")
        print(f"  - 总文档数: {stats['total_documents']}")
        print(f"  - 向量维度: {stats['embedding_dimension']}")
        print(f"  - 章节数: {stats['total_sections']}")

        # 主循环: 持续接收用户输入
        while True:
            try:
                # 提示用户输入查询
                user_input = input("\n请输入查询 (输入 'help' 查看帮助): ").strip()

                if not user_input:  # 跳过空输入
                    continue

                parts = user_input.split()
                command = parts[0].lower()

                # 执行搜索
                query = user_input
                print(f"\n搜索: '{query}'")

                # 调用搜索器,返回前5个结果
                results = searcher.search(query, top_k=5)
                searcher.print_search_results(results, query)

            except KeyboardInterrupt:  # Ctrl+C退出
                break
            except Exception as e:
                print(f"发生错误: {e}")

    except Exception as e:
        print(f"初始化失败: {e}")
        print("请确保已运行 vehicle_data_processor.py 生成向量数据库")
        return 1

    return 0
      while True:
           try:
                # 提示用户输入查询
                user_input = input("\n请输入查询 (输入 'help' 查看帮助): ").strip()

                if not user_input:  # 跳过空输入
                    continue

                parts = user_input.split()
                command = parts[0].lower()

                # 执行搜索
                query = user_input
                print(f"\n搜索: '{query}'")

                # 调用搜索器,返回前5个结果
                results = searcher.search(query, top_k=5)
                searcher.print_search_results(results, query)

            except KeyboardInterrupt:  # Ctrl+C退出
                break
            except Exception as e:
                print(f"发生错误: {e}")

    except Exception as e:
        print(f"初始化失败: {e}")
        print("请确保已运行 vehicle_data_processor.py 生成向量数据库")
        return 1

    return 0

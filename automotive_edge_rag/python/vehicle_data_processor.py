#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import json
import pickle
import numpy as np
from typing import List, Dict, Any, Tuple
from sentence_transformers import SentenceTransformer
from sklearn.metrics.pairwise import cosine_similarity
import logging

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)


class VehicleDataProcessor:
    """
    车辆数据处理器 - 离线使用,将车辆手册文本转换为向量数据库

    主要功能:
    1. 解析车辆手册文本(Markdown格式)
    2. 将文本分割成章节和子章节
    3. 使用SentenceTransformer模型生成文本向量
    4. 保存向量数据库供后续检索使用
    """

    def __init__(self, model_name):
        """
        初始化数据处理器

        Args:
            model_name: 文本向量化模型的名称或路径
        """
        self.model_name = model_name
        self.model = None  # SentenceTransformer模型(延迟加载)
        self.embeddings = None  # 生成的向量数组
        self.texts = []  # 处理后的文本列表
        self.metadata = []  # 文本的元数据

        # 创建输出目录
        os.makedirs("vector_db", exist_ok=True)  # 向量数据库目录
        os.makedirs("processed_data", exist_ok=True)  # 处理后的数据目录

    def load_model(self):
        """
        加载文本向量化模型

        使用SentenceTransformer加载预训练模型,用于将文本转换为向量
        """
        try:
            logger.info(f"正在加载模型: {self.model_name}")
            self.model = SentenceTransformer(self.model_name)
            logger.info("模型加载成功")
        except Exception as e:
            logger.error(f"模型加载失败: {e}")
            raise

    def load_vehicle_data(self, file_path: str) -> List[Dict[str, Any]]:
        """
        加载车辆手册数据文件

        Args:
            file_path: 车辆手册文本文件路径(通常是Markdown格式)

        Returns:
            解析后的章节列表,每个章节包含section, subsection, content等字段
        """
        logger.info(f"正在加载数据文件: {file_path}")

        # 读取整个文件内容
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()

        # 解析成结构化数据
        sections = self._parse_vehicle_manual(content)
        return sections

    def _parse_vehicle_manual(self, content: str) -> List[Dict[str, Any]]:
        """
        解析车辆手册文本,按章节和子章节分割

        文本格式:
        ## 主章节标题
        内容...
        ### 子章节标题
        内容...

        Args:
            content: 完整的手册文本

        Returns:
            分割后的章节列表
        """
        sections = []
        lines = content.split('\n')

        # 状态变量:跟踪当前章节、子章节和内容
        current_section = None
        current_subsection = None
        current_content = []

        for line in lines:
            line = line.strip()
            if not line:  # 跳过空行
                continue

            # 检测主章节标题 (##)
            if line.startswith('## '):
                # 保存上一个章节的内容
                if current_section:
                    sections.append({
                        'section': current_section,
                        'subsection': current_subsection,
                        'content': '\n'.join(current_content),
                        'type': 'section'
                    })

                # 开始新的主章节
                current_section = line[3:]  # 去掉"## "前缀
                current_subsection = None
                current_content = []

            # 检测子章节标题 (###)
            elif line.startswith('### '):
                # 保存上一个子章节的内容
                if current_subsection and current_content:
                    sections.append({
                        'section': current_section,
                        'subsection': current_subsection,
                        'content': '\n'.join(current_content),
                        'type': 'subsection'
                    })

                # 开始新的子章节
                current_subsection = line[4:]  # 去掉"### "前缀
                current_content = []

            else:
                # 普通文本行,添加到当前内容
                current_content.append(line)

        # 保存最后一个章节
        if current_section and current_content:
            sections.append({
                'section': current_section,
                'subsection': current_subsection,
                'content': '\n'.join(current_content),
                'type': 'subsection' if current_subsection else 'section'
            })

        logger.info(f"解析完成,共生成 {len(sections)} 个文本片段")
        return sections

    def prepare_texts_for_embedding(self, data: List[Dict[str, Any]]) -> Tuple[List[str], List[Dict[str, Any]]]:
        """
        准备文本用于向量化

        将章节信息和内容组合成完整的文本,方便后续搜索时匹配
        例如: "章节: 发动机系统 | 子章节: 故障处理 | 内容: ..."

        Args:
            data: 解析后的章节数据列表

        Returns:
            (texts, metadata) - 组合后的文本列表和对应的元数据
        """
        texts = []
        metadata = []

        for i, item in enumerate(data):
            # 组合章节标题和内容
            text_parts = []
            if item['section']:
                text_parts.append(f"章节: {item['section']}")
            if item['subsection']:
                text_parts.append(f"子章节: {item['subsection']}")
            if item['content']:
                text_parts.append(item['content'])

            # 用" | "分隔符连接各部分
            text = " | ".join(text_parts)
            texts.append(text)

            # 构建元数据
            meta = {
                'id': i,  # 文档ID
                'section': item['section'],  # 章节名
                'subsection': item['subsection'],  # 子章节名
                'type': item['type'],  # 类型(section/subsection)
                'content_length': len(item['content'])  # 内容长度
            }
            metadata.append(meta)

        return texts, metadata

    def generate_embeddings(self, texts: List[str]) -> np.ndarray:
        """
        生成文本向量

        使用SentenceTransformer模型将文本列表转换为向量矩阵
        例如: 100个文本 -> (100, 768)的向量矩阵

        Args:
            texts: 文本列表

        Returns:
            向量矩阵,形状为(文本数量, 向量维度)
        """
        # 如果模型未加载,先加载
        if self.model is None:
            self.load_model()

        logger.info(f"正在为 {len(texts)} 个文本生成向量...")
        # encode()方法将文本批量转换为向量
        embeddings = self.model.encode(texts, show_progress_bar=True)
        logger.info(f"向量生成完成,形状: {embeddings.shape}")

        return embeddings

    def save_vector_database(self, embeddings: np.ndarray, texts: List[str], metadata: List[Dict[str, Any]]):
        """
        保存向量数据库到磁盘

        保存三个文件:
        1. vehicle_embeddings.npy - 向量数据(numpy二进制格式,加载快)
        2. vehicle_data.pkl - 文本和元数据(pickle序列化)
        3. vehicle_data.json - 文本和元数据(JSON格式,方便查看)

        Args:
            embeddings: 向量矩阵
            texts: 文本列表
            metadata: 元数据列表
        """
        # 保存向量数据(.npy格式,numpy专用,加载速度快)
        vector_file = "vector_db/vehicle_embeddings.npy"
        np.save(vector_file, embeddings)
        logger.info(f"向量数据已保存到: {vector_file}")

        # 保存文本和元数据(.pkl格式,pickle序列化Python对象)
        data_file = "vector_db/vehicle_data.pkl"
        data = {
            'texts': texts,  # 所有文本
            'metadata': metadata,  # 所有元数据
            'model_name': self.model_name  # 记录使用的模型名
        }
        with open(data_file, 'wb') as f:
            pickle.dump(data, f)
        logger.info(f"文本和元数据已保存到: {data_file}")

        # 同时保存为JSON格式(方便人工查看和调试)
        json_file = "vector_db/vehicle_data.json"
        json_data = {
            'texts': texts,
            'metadata': metadata,
            'model_name': self.model_name,
            'embedding_shape': embeddings.shape  # 记录向量形状
        }
        with open(json_file, 'w', encoding='utf-8') as f:
            json.dump(json_data, f, ensure_ascii=False, indent=2)
        logger.info(f"JSON格式数据已保存到: {json_file}")

    def create_search_index(self, embeddings: np.ndarray, texts: List[str], metadata: List[Dict[str, Any]]):
        """
        创建搜索索引

        预计算相似度矩阵和索引信息,加速后续检索

        Args:
            embeddings: 向量矩阵
            texts: 文本列表
            metadata: 元数据列表
        """
        # 预计算所有文档之间的相似度矩阵(可选,用于加速某些查询)
        # 例如: 100个文档 -> (100, 100)的相似度矩阵
        similarity_matrix = cosine_similarity(embeddings)

        # 保存相似度矩阵
        similarity_file = "vector_db/similarity_matrix.npy"
        np.save(similarity_file, similarity_matrix)
        logger.info(f"相似度矩阵已保存到: {similarity_file}")

        # 创建索引信息文件(记录数据库的基本信息)
        index_info = {
            'total_documents': len(texts),  # 文档总数
            'embedding_dimension': embeddings.shape[1],  # 向量维度
            'model_name': self.model_name,  # 模型名称
            'created_at': str(np.datetime64('now'))  # 创建时间
        }

        index_file = "vector_db/index_info.json"
        with open(index_file, 'w', encoding='utf-8') as f:
            json.dump(index_info, f, ensure_ascii=False, indent=2)
        logger.info(f"索引信息已保存到: {index_file}")

    def process_vehicle_data(self, input_file: str):
        """
        完整的数据处理流程 - 主函数

        流程:
        1. 加载车辆手册文本
        2. 解析并准备文本
        3. 生成向量
        4. 保存向量数据库
        5. 创建搜索索引

        Args:
            input_file: 输入的车辆手册文本文件路径
        """
        logger.info("开始处理车载数据...")

        # 步骤1: 加载并解析数据
        data = self.load_vehicle_data(input_file)

        # 步骤2: 准备用于向量化的文本
        texts, metadata = self.prepare_texts_for_embedding(data)

        # 步骤3: 生成文本向量
        embeddings = self.generate_embeddings(texts)

        # 步骤4: 保存向量数据库文件
        self.save_vector_database(embeddings, texts, metadata)

        # 步骤5: 创建搜索索引
        self.create_search_index(embeddings, texts, metadata)

        logger.info("车载数据处理完成!")

        # 输出统计信息
        logger.info(f"处理统计:")
        logger.info(f"  - 文本片段数量: {len(texts)}")
        logger.info(f"  - 向量维度: {embeddings.shape[1]}")
        logger.info(
            f"  - 章节数量: {len(set(m['section'] for m in metadata if m['section']))}")
        logger.info(
            f"  - 子章节数量: {len(set(m['subsection'] for m in metadata if m['subsection']))}")


def main():
    """
    主函数 - 脚本入口

    用法: python vehicle_data_processor.py
    功能: 将vehicle_manual_data.txt转换为向量数据库
    """
    # 创建数据处理器
    processor = VehicleDataProcessor()

    # 处理车辆手册数据
    input_file = "vehicle_manual_data.txt"
    processor.process_vehicle_data(input_file)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import json
import pickle
import time
import numpy as np
from typing import List, Dict, Any, Tuple
from sentence_transformers import SentenceTransformer
from sklearn.metrics.pairwise import cosine_similarity
import logging

# 配置日志
logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)


class VehicleVectorSearch:
    """
    车载向量搜索类 - 用于基于向量相似度搜索车辆手册内容

    功能:
    1. 加载预先生成的向量数据库
    2. 加载文本向量化模型(SentenceTransformer)
    3. 将用户问题转换成向量并搜索最相似的内容
    """

    def __init__(self, model_path, vector_db_path: str = "vector_db"):
        """
        初始化搜索器

        Args:
            model_path: 文本向量化模型的路径
            vector_db_path: 向量数据库文件夹路径
        """
        self.vector_db_path = vector_db_path
        self.model = None  # 文本向量化模型(延迟加载)
        self.model_path = model_path
        self.embeddings = None  # 所有文档的向量数组
        self.texts = []  # 原始文本列表
        self.metadata = []  # 元数据(章节名、类型等)
        self.similarity_matrix = None  # 相似度矩阵(可选)

        # 加载向量数据库
        self._load_vector_database()

    def _load_vector_database(self):
        """
        加载向量数据库到内存

        加载三个文件:
        1. vehicle_embeddings.npy - 所有文档的向量数据
        2. vehicle_data.pkl - 原始文本和元数据
        3. similarity_matrix.npy - 预计算的相似度矩阵(可选)
        """
        try:
            # 步骤1: 加载向量数据(.npy文件是numpy数组格式)
            embeddings_file = os.path.join(
                self.vector_db_path, "vehicle_embeddings.npy")
            if os.path.exists(embeddings_file):
                self.embeddings = np.load(embeddings_file)
                logger.info(f"向量数据加载成功，形状: {self.embeddings.shape}")
            else:
                raise FileNotFoundError(f"向量文件不存在: {embeddings_file}")

            # 步骤2: 加载文本和元数据
            data_file = os.path.join(self.vector_db_path, "vehicle_data.pkl")
            if os.path.exists(data_file):
                with open(data_file, 'rb') as f:
                    data = pickle.load(f)  # pickle用于序列化Python对象
                self.texts = data['texts']  # 所有文档文本
                self.metadata = data['metadata']  # 每个文档的元信息
                logger.info(f"文本和元数据加载成功，共 {len(self.texts)} 个文档")
            else:
                raise FileNotFoundError(f"数据文件不存在: {data_file}")

            # 步骤3: 加载相似度矩阵(可选,用于加速搜索)
            similarity_file = os.path.join(
                self.vector_db_path, "similarity_matrix.npy")
            if os.path.exists(similarity_file):
                self.similarity_matrix = np.load(similarity_file)
                logger.info("相似度矩阵加载成功")

        except Exception as e:
            logger.error(f"加载向量数据库失败: {e}")
            raise

    def load_model(self, model_name):
        """
        加载文本向量化模型(SentenceTransformer)

        该模型能将文本转换为向量,例如:
        "发动机故障" -> [0.123, -0.456, 0.789, ...] (768维向量)

        Args:
            model_name: 模型路径或名称
        """
        try:
            logger.info(f"正在加载模型: {model_name}")

            model_paths = [
                model_name  # 可以扩展多个后备路径
            ]

            model_loaded = False
            for path in model_paths:
                try:
                    logger.info(f"尝试加载模型: {path}")
                    # 加载SentenceTransformer模型(基于BERT/RoBERTa等)
                    self.model = SentenceTransformer(path)
                    logger.info(f"模型加载成功: {path}")
                    model_loaded = True
                    break
                except Exception as e:
                    logger.warning(f"模型加载失败: {path} - {e}")
                    continue

            if not model_loaded:
                raise Exception("所有模型路径都无法加载")

        except Exception as e:
            logger.error(f"模型加载失败: {e}")
            raise

    def search(self, query: str, top_k: int = 5, threshold: float = 0.5) -> List[Dict[str, Any]]:
        """
        搜索与查询最相关的文档

        流程:
        1. 将查询文本转换为向量
        2. 计算与所有文档向量的余弦相似度
        3. 返回相似度最高的top_k个结果

        Args:
            query: 用户问题
            top_k: 返回前几个结果
            threshold: 相似度阈值(0-1),低于该值的结果不返回

        Returns:
            搜索结果列表,每个结果包含id, text, metadata, similarity等信息
        """
        # 如果模型还未加载,先加载(延迟加载)
        if self.model is None:
            self.load_model(self.model_path)

        # 步骤1: 将问题转换成向量
        query_embedding = self.model.encode([query])  # 返回形状: (1, 768)

        # 步骤2: 计算与所有文档的余弦相似度
        # cosine_similarity计算公式: cos(θ) = (A·B) / (||A|| * ||B||)
        # 相似度范围: -1到1, 1表示完全相同
        similarities = cosine_similarity(query_embedding, self.embeddings)[0]

        # 步骤3: 找到相似度最高的top_k个索引
        # argsort()升序排序, [::-1]反转为降序, [:top_k]取前k个
        top_indices = np.argsort(similarities)[::-1][:top_k]

        # 步骤4: 组装结果
        results = []
        for idx in top_indices:
            similarity = similarities[idx]
            # 只返回相似度高于阈值的结果
            if similarity >= threshold:
                result = {
                    'id': idx,  # 文档ID
                    'text': self.texts[idx],  # 原始文本
                    'metadata': self.metadata[idx],  # 元数据
                    'similarity': float(similarity),  # 相似度分数
                    'section': self.metadata[idx]['section'],  # 章节名
                    'subsection': self.metadata[idx]['subsection']  # 子章节名
                }
                results.append(result)

        return results

    def get_statistics(self) -> Dict[str, Any]:
        """
        获取向量数据库的统计信息

        Returns:
            统计信息字典,包含文档数、向量维度、章节数等
        """
        # 统计不同的章节和子章节
        sections = set(m['section'] for m in self.metadata if m['section'])
        subsections = set(m['subsection']
                          for m in self.metadata if m['subsection'])

        return {
            'total_documents': len(self.texts),  # 总文档数
            # 向量维度
            'embedding_dimension': self.embeddings.shape[1] if self.embeddings is not None else 0,
            'total_sections': len(sections),  # 章节数
            'total_subsections': len(subsections),  # 子章节数
            'sections': list(sections),  # 章节列表
            'has_similarity_matrix': self.similarity_matrix is not None  # 是否有相似度矩阵
        }

    def print_search_results(self, results: List[Dict[str, Any]], query: str = ""):
        if query:
            print(f"\n查询: '{query}'")

        if not results:
            print("未找到相关结果")
            return

        print(f"\n找到 {len(results)} 个相关结果:")
        print("-" * 80)

        for i, result in enumerate(results, 1):
            print(f"\n{i}. 相似度: {result['similarity']:.4f}")
            print(f"   章节: {result['section']}")
            if result['subsection']:
                print(f"   子章节: {result['subsection']}")
            print(f"   内容: {result['text'][:200]}...")
            print("-" * 40)


def main():
    base_dir = os.path.dirname(os.path.abspath(__file__))
    model_path = os.path.join(base_dir, "..", "models")
    searcher = VehicleVectorSearch(os.path.normpath(model_path))

    stats = searcher.get_statistics()
    print("向量数据库统计信息:")
    print(f"  - 总文档数: {stats['total_documents']}")
    print(f"  - 向量维度: {stats['embedding_dimension']}")
    print(f"  - 章节数: {stats['total_sections']}")
    print(f"  - 子章节数: {stats['total_subsections']}")

    test_queries = [
        "发动机故障",
        "制动系统",
        "保养周期",
        "安全气囊",
        "燃油经济性",
        "天气怎么样"
    ]

    print("\n" + "="*80)
    print("搜索演示")
    print("="*80)

    for query in test_queries:
        start_time = time.time()

        results = searcher.search(query, top_k=1)
        searcher.print_search_results(results, query)
        end_time = time.time()

        print(f"数据处理完成！耗时: {end_time - start_time:.2f} 秒")


if __name__ == "__main__":
    main()

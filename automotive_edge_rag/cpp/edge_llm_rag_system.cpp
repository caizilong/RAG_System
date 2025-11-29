#pragma once
#include "edge_llm_rag_system.h"

#include <chrono>
#include <codecvt>
#include <cstdlib>
#include <cwchar>
#include <iomanip>
#include <iostream>
#include <random>
#include <regex>
#include <thread>

#include "query_classifier.h"

namespace edge_llm_rag {
// 构造函数: 初始化RAG系统,加载Python向量搜索模块和模型
EdgeLLMRAGSystem::EdgeLLMRAGSystem() : is_initialized_(false) {
    // 步骤1: 配置Python路径,让C++能找到Python脚本
    py::module_ sys = py::module_::import("sys");
    py::list path   = sys.attr("path");
    path.append("python");  // 添加python目录到搜索路径

    // 步骤2: 导入Python向量搜索模块,创建搜索器对象
    py::module_ mod = py::module_::import("vehicle_vector_search");
    searcher        = mod.attr("VehicleVectorSearch")("vector_db");  // 指定向量数据库路径

    // 步骤3: 加载文本向量化模型(用于将问题转换成向量)
    std::cout << "Loading model once..." << std::endl;
    auto load_t0 = std::chrono::high_resolution_clock::now();  // 记录开始时间

    fs::path cpp_dir    = fs::absolute(__FILE__).parent_path();
    fs::path model_path = cpp_dir.parent_path() / "models";  // 模型路径: ../models
    searcher.attr("load_model")(model_path.string());        // 调用Python的load_model方法

    auto load_t1   = std::chrono::high_resolution_clock::now();  // 记录结束时间
    double load_ms = std::chrono::duration<double, std::milli>(load_t1 - load_t0).count();
    std::cout << "Model loaded (" << std::fixed << std::setprecision(2) << load_ms << " ms)"
              << std::endl;

    // 步骤4: 打印向量数据库统计信息(文档总数、向量维度等)
    py::object stats = searcher.attr("get_statistics")();
    std::cout << "Stats: total_documents=" << stats["total_documents"].cast<int>()
              << ", embedding_dimension=" << stats["embedding_dimension"].cast<int>() << std::endl;
}

EdgeLLMRAGSystem::~EdgeLLMRAGSystem() {}

// 初始化系统: 创建查询分类器和清空缓存
bool EdgeLLMRAGSystem::initialize() {
    try {
        // 创建查询分类器(用于判断问题类型:紧急、事实、创意等)
        query_classifier_ = std::make_unique<QueryClassifier>();

        // 清空查询缓存
        query_cache_.clear();

        is_initialized_ = true;
        std::cout << "系统初始化成功" << std::endl;
        return true;
    } catch (const std::exception &e) {
        std::cerr << "系统初始化失败: " << e.what() << std::endl;
        return false;
    }
}

// 核心处理函数: 根据问题类型选择合适的回答方式
std::string EdgeLLMRAGSystem::process_query(const std::string &query, const std::string &user_id,
                                            const std::string &context) {
    // 检查系统是否已初始化
    if (!is_initialized_) {
        return "系统未初始化";
    }

    // 步骤1: 先查缓存,如果之前问过相同问题直接返回
    std::string cached_response = get_from_cache(query);
    if (!cached_response.empty()) {
        return cached_response;  // 缓存命中,直接返回
    }

    // 步骤2: 对问题进行分类(紧急、事实、创意、复杂、未知)
    auto classification = classify_query(query);

    // 步骤3: 根据分类结果选择不同的回答策略
    std::string response;
    switch (classification.query_type) {
        case QueryClassification::EMERGENCY_QUERY:  // 紧急问题(如"发动机故障")
            std::cout << "===============================" << std::endl;
            std::cout << "紧急查询 detected, using RAG only response." << std::endl;
            std::cout << "===============================" << std::endl;
            response = rag_only_response(query);  // 只用RAG,直接查手册
            break;
        case QueryClassification::FACTUAL_QUERY:  // 事实性问题(如"保养周期")
            std::cout << "===============================" << std::endl;
            std::cout << "事实性查询 detected, using RAG only response." << std::endl;
            std::cout << "===============================" << std::endl;
            response = rag_only_response(query);  // 只用RAG,查手册更准确
            break;
        case QueryClassification::COMPLEX_QUERY:  // 复杂问题(需要综合分析)
            std::cout << "===============================" << std::endl;
            std::cout << "复杂查询 detected, using hybrid response." << std::endl;
            std::cout << "===============================" << std::endl;

            response = hybrid_response(query);  // 混合模式:RAG+LLM综合回答
            break;
        case QueryClassification::CREATIVE_QUERY:  // 创意问题(如"推荐旅游路线")
            std::cout << "===============================" << std::endl;
            std::cout << "创意查询 detected, using LLM only response." << std::endl;
            std::cout << "===============================" << std::endl;
            response = llm_only_response(query);  // 只用LLM,生成创意内容
            break;
        default:  // 未知类型
            std::cout << "===============================" << std::endl;
            std::cout << "未知查询类型, using adaptive response." << std::endl;
            std::cout << "===============================" << std::endl;
            response = hybrid_response(query);  // 默认用混合模式
    }

    // 步骤4: 将问题和答案加入缓存
    add_to_cache(query, response);

    return response;
}

QueryClassification EdgeLLMRAGSystem::classify_query(const std::string &query) {
    if (!query_classifier_) {
        return QueryClassification{QueryClassification::UNKNOWN_QUERY, 0.0f, "分类器未初始化",
                                   false};
    }

    return query_classifier_->classify_query(query);
}

// 将RAG答案按句子分割后发送给TTS(文字转语音)
void EdgeLLMRAGSystem::rag_message_worker(const std::string &rag_text) {
    // 定义句子分隔符:句号、问号、感叹号等
    static const std::wregex wide_delimiter(L"([。！？；：\n]|\\?\\s|\\!\\s|\\；|\\，|\\、|\\|)");
    const std::wstring END_MARKER = L"END";

    // UTF-8转宽字符(处理中文)
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

    std::wstring wide_text = converter.from_bytes(rag_text) + END_MARKER;

    // 找到所有句子分隔符的位置
    std::wsregex_iterator it(wide_text.begin(), wide_text.end(), wide_delimiter);
    std::wsregex_iterator end;

    // 跳过前两个分隔符(避免开头的短句)
    int skip_counter = 0;
    size_t last_pos  = 0;
    while (it != end && skip_counter < 2) {
        last_pos = it->position() + it->length();
        ++it;
        ++skip_counter;
    }

    // 逐句提取并发送给TTS
    while (it != end) {
        size_t seg_start = last_pos;
        size_t seg_end   = it->position();
        last_pos         = seg_end + it->length();

        // 提取一个句子
        std::wstring wide_segment = wide_text.substr(seg_start, seg_end - seg_start);

        // 去除前后空白字符
        wide_segment.erase(0, wide_segment.find_first_not_of(L" \t\n\r"));
        wide_segment.erase(wide_segment.find_last_not_of(L" \t\n\r") + 1);

        if (!wide_segment.empty()) {
            // 转换回UTF-8并发送给TTS服务进行语音合成
            auto response1 = tts_client_.request(converter.to_bytes(wide_segment));
            std::cout << "[tts -> RAG] received: " << response1 << std::endl;
        }
        ++it;
    }

    // 处理最后剩余的内容
    if (last_pos < wide_text.length()) {
        std::wstring last_segment = wide_text.substr(last_pos);
        if (!last_segment.empty()) {
            auto response1 = tts_client_.request(converter.to_bytes(last_segment));
            std::cout << "[tts -> RAG] received: " << response1 << std::endl;
        }
    }
}

// RAG模式: 只从向量数据库检索答案,不使用LLM
std::string EdgeLLMRAGSystem::rag_only_response(const std::string &query, bool preload) {
    // 计时开始
    auto t0 = std::chrono::high_resolution_clock::now();

    // 调用Python搜索器: top_k=1(返回1个结果), threshold=0.5(相似度阈值)
    py::object results = searcher.attr("search")(query, 1, 0.5);

    auto t1   = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::cout << "\nQuery: '" << query << "'\n";
    std::cout << "elapsed: " << std::fixed << std::setprecision(2) << ms << " ms\n";

    // 检查是否找到结果
    if (py::len(results) == 0) {
        std::cout << "  No results" << std::endl;
        return "No results !!!";
    }

    // 提取搜索结果
    std::string answer;
    for (const auto &item : results) {
        double sim             = item["similarity"].cast<double>();  // 相似度分数
        std::string text       = item["text"].cast<std::string>();   // 文本内容
        answer                 = item["text"].cast<std::string>();
        std::string section    = item["section"].cast<std::string>();     // 章节名
        std::string subsection = item["subsection"].cast<std::string>();  // 子章节名

        // 打印搜索结果摘要
        std::cout << "  sim=" << std::fixed << std::setprecision(4) << sim
                  << ", section=" << section << (subsection.empty() ? "" : ("/" + subsection))
                  << ", text=" << text.substr(0, 100) << "...\n";
    }

    // 如果不是预加载模式,将答案发送给TTS进行语音播报
    if (!preload) {
        rag_message_worker(answer);
    }

    return answer;
}

// LLM模式: 只使用大语言模型生成答案,不查询向量库
std::string EdgeLLMRAGSystem::llm_only_response(const std::string &query) {
    // 直接将问题发送给LLM服务(通过ZMQ通信)
    auto response = llm_client_.request(query);
    std::cout << "[tts -> RAG] received: " << response << std::endl;
    return response;
}

// 混合模式: 先用RAG检索相关信息,再让LLM基于这些信息生成答案
std::string EdgeLLMRAGSystem::hybrid_response(const std::string &query) {
    // 步骤1: 先从向量库检索相关信息(preload=true表示不播报)
    std::string rag_part = rag_only_response(query, true);

    // 步骤2: 如果RAG没找到相关内容,直接用LLM回答
    if (rag_part.find("No results") != std::string::npos) {
        return llm_only_response(query);
    }

    // 步骤3: 将问题和RAG检索的内容一起发给LLM
    // 格式: "用户问题<rag>手册相关内容"
    std::string llm_query = query + "<rag>" + rag_part;
    std::string llm_part  = llm_only_response(llm_query);

    return llm_part;  // 返回LLM综合生成的答案
}

// 添加查询结果到缓存
bool EdgeLLMRAGSystem::add_to_cache(const std::string &query, const std::string &response) {
    // 如果缓存超过100条,清空缓存(避免内存占用过大)
    if (query_cache_.size() >= 100) {
        query_cache_.clear();
    }

    // 保存问题和答案的映射
    query_cache_[query] = response;
    return true;
}

// 从缓存中获取答案
std::string EdgeLLMRAGSystem::get_from_cache(const std::string &query) {
    auto it = query_cache_.find(query);
    if (it != query_cache_.end())  // 缓存命中
    {
        return it->second;  // 返回缓存的答案
    }
    return "";  // 缓存未命中,返回空字符串
}

bool EdgeLLMRAGSystem::is_cache_valid(const std::string &query) {
    return query_cache_.find(query) != query_cache_.end();
}

bool EdgeLLMRAGSystem::preload_common_queries() {
    // 预加载常用查询
    std::vector<std::string> common_queries = {"发动机故障", "制动系统", "空调不制冷", "保养周期"};

    for (const auto &query : common_queries) {
        if (query_cache_.find(query) == query_cache_.end()) {
            add_to_cache(query, rag_only_response(query, true));
        }
    }

    return true;
}

}  // namespace edge_llm_rag
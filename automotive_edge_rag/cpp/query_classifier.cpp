#include "query_classifier.h"

#include <algorithm>
#include <iostream>
#include <regex>

namespace edge_llm_rag {

// 构造函数: 初始化关键词字典
QueryClassifier::QueryClassifier() {
    initialize_keyword_dictionary();  // 加载各类关键词(紧急、技术、保养、创意等)
}

// 分析查询特征: 提取关键词并计算各类分数
QueryFeatures QueryClassifier::analyze_query_features(const std::string &query) {
    QueryFeatures features;
    features.query_length = static_cast<int>(query.length());  // 记录问题长度

    // 步骤1: 提取关键词(在问题中查找预定义的关键词)
    features.keywords = extract_keywords(query);

    // 步骤2: 计算各类分数
    features.urgency_score = calculate_urgency_score(features.keywords);  // 紧急度(0-1)
    features.complexity_score =
        calculate_complexity_score(query, features.keywords);               // 复杂度(0-1)
    features.factual_score  = calculate_factual_score(features.keywords);   // 事实性(0-1)
    features.creative_score = calculate_creative_score(features.keywords);  // 创意性(0-1)

    // 步骤3: 检测特殊类型词汇
    features.contains_question_words  = detect_question_words(query);   // 是否含有疑问词
    features.contains_emergency_words = detect_emergency_words(query);  // 是否含有紧急词
    features.contains_technical_words = detect_technical_words(query);  // 是否含有技术词

    return features;
}

// 核心分类函数: 根据特征分数决定问题类型
QueryClassification QueryClassifier::classify_query(const std::string &query) {
    // 分析问题特征
    QueryFeatures features = analyze_query_features(query);

    QueryClassification classification;

    // 判断是否需要立即响应(紧急度>0.7)
    classification.requires_immediate_response = features.urgency_score > 0.7f;

    // 分类逻辑(按优先级排序):
    if (features.urgency_score > 0.7f || features.contains_emergency_words) {
        // 1. 紧急查询: 故障、危险等紧急情况
        classification.query_type = QueryClassification::EMERGENCY_QUERY;
    } else if (features.factual_score >= 0.5f) {
        // 2. 事实性查询: 查询车辆技术参数、保养信息等
        classification.query_type = QueryClassification::FACTUAL_QUERY;
    } else if (features.creative_score > 0.6f) {
        // 3. 创意查询: 旅游推荐、闲聊等
        classification.query_type = QueryClassification::CREATIVE_QUERY;
    } else if (features.complexity_score > 0.6f) {
        // 4. 复杂查询: 需要综合分析的问题
        classification.query_type = QueryClassification::COMPLEX_QUERY;
    } else {
        // 5. 未知查询: 无法明确分类
        classification.query_type = QueryClassification::UNKNOWN_QUERY;
    }

    return classification;
}

// 初始化关键词字典: 定义各种类型的关键词
void QueryClassifier::initialize_keyword_dictionary() {
    // 1. 紧急类关键词: 故障、危险等需要立即处理的情况
    keyword_dict_["emergency"] = {"故障",     "警告",     "危险",     "紧急",       "异常",
                                  "失灵",     "失效",     "损坏",     "发动机故障", "制动故障",
                                  "转向故障", "电气故障", "安全气囊", "ABS故障"};

    // 2. 技术类关键词: 车辆系统和部件名称
    keyword_dict_["technical"] = {"发动机", "制动",     "变速箱", "电气",   "空调",
                                  "转向",   "悬挂",     "轮胎",   "机油",   "冷却液",
                                  "制动液", "变速箱油", "电瓶",   "发电机", "起动机"};

    // 3. 保养类关键词: 维修保养相关
    keyword_dict_["maintenance"] = {"保养",   "维修",   "更换",   "检查",     "清洁",
                                    "调整",   "润滑",   "紧固",   "定期保养", "机油更换",
                                    "滤清器", "火花塞", "制动片", "轮胎更换"};

    // 4. 功能类关键词: 车辆功能和设备
    keyword_dict_["feature"] = {"自动泊车", "车道保持", "定速巡航", "导航", "娱乐", "空调控制",
                                "座椅调节", "后视镜",   "雨刷",     "灯光", "音响", "蓝牙"};

    // 5. 疑问词: 用于判断是否是问句
    keyword_dict_["question"] = {"什么",     "怎么",   "如何",     "为什么", "哪里",   "何时",
                                 "多少",     "哪个",   "吗",       "呢",     "嘛",     "能不能",
                                 "可不可以", "有没有", "推荐一下", "怎么去", "去哪里", "怎么玩"};

    // 6. 创意类关键词: 旅游、娱乐、生活服务等
    keyword_dict_["creative"] = {
        "推荐",     "建议",   "想法",   "创意",     "优化",     "改进",   "设计",   "规划",
        "旅游",     "旅行",   "出行",   "景点",     "门票",     "酒店",   "民宿",   "机票",
        "火车票",   "高铁",   "行程",   "路线",     "攻略",     "签证",   "租车",   "自驾",
        "海岛",     "海滩",   "公园",   "博物馆",   "古镇",     "温泉",   "夜市",   "特产",
        "美食",     "摄影",   "网红",   "打卡",     "露营",     "徒步",   "游玩",   "娱乐",
        "主题乐园", "游乐园", "迪士尼", "环球影城", "水上乐园", "演唱会", "音乐节", "展览",
        "赛事",     "滑雪",   "潜水",   "骑行",     "登山",     "预订",   "订票",   "订酒店",
        "退改签",   "行李",   "登机",   "值机",     "改签",     "延误",   "转机",   "天气",
        "笑话",     "故事",   "新闻",   "百科",     "科普",     "翻译",   "计算",   "单位换算",
        "今天",     "明天",   "现在",   "附近",     "哪里有",   "怎么走"};
}

// 提取关键词: 在问题中查找所有预定义的关键词
std::vector<std::string> QueryClassifier::extract_keywords(const std::string &query) {
    std::vector<std::string> keywords;

    // 遍历所有关键词字典
    for (const auto &[category, words] : keyword_dict_) {
        // 在问题中查找每个关键词
        for (const auto &word : words) {
            if (query.find(word) != std::string::npos)  // 如果找到
            {
                keywords.push_back(word);  // 添加到结果列表
            }
        }
    }

    return keywords;
}

// 计算紧急度分数: 统计紧急关键词的数量
float QueryClassifier::calculate_urgency_score(const std::vector<std::string> &keywords) {
    float score         = 0.0f;
    int emergency_count = 0;

    // 统计紧急关键词出现次数
    for (const auto &keyword : keywords) {
        if (std::find(keyword_dict_["emergency"].begin(), keyword_dict_["emergency"].end(),
                      keyword) != keyword_dict_["emergency"].end()) {
            emergency_count++;
        }
    }

    // 每个紧急词加0.3分,最高1.0
    score = std::min(1.0f, static_cast<float>(emergency_count) * 0.3f);
    return score;
}

// 计算复杂度分数: 综合问题长度、关键词数量、技术词汇比例
float QueryClassifier::calculate_complexity_score(const std::string &query,
                                                  const std::vector<std::string> &keywords) {
    float score = 0.0f;

    // 因素1: 查询长度，占怰30%，超过100字符计为1.0
    score += std::min(1.0f, static_cast<float>(query.length()) / 100.0f) * 0.3f;

    // 因素2: 关键词数量，占怰40%，10个关键词以上计为1.0
    score += std::min(1.0f, static_cast<float>(keywords.size()) / 10.0f) * 0.4f;

    // 因素3: 技术词汇比例，占怰30%，5个技术词以上计为1.0
    int technical_count = 0;
    for (const auto &keyword : keywords) {
        if (std::find(keyword_dict_["technical"].begin(), keyword_dict_["technical"].end(),
                      keyword) != keyword_dict_["technical"].end()) {
            technical_count++;
        }
    }
    score += std::min(1.0f, static_cast<float>(technical_count) / 5.0f) * 0.3f;

    return std::min(1.0f, score);  // 最终分数不超过1.0
}

// 计算事实性分数: 统计技术、保养、功能类关键词
float QueryClassifier::calculate_factual_score(const std::vector<std::string> &keywords) {
    float score = 0.0f;

    for (const auto &keyword : keywords) {
        // 技术类关键词: 每个加0.4分
        if (std::find(keyword_dict_["technical"].begin(), keyword_dict_["technical"].end(),
                      keyword) != keyword_dict_["technical"].end()) {
            score += 0.4f;
        }
        // 保养类关键词: 每个加0.4分
        if (std::find(keyword_dict_["maintenance"].begin(), keyword_dict_["maintenance"].end(),
                      keyword) != keyword_dict_["maintenance"].end()) {
            score += 0.4f;
        }
        // 功能类关键词: 每个加0.5分
        if (std::find(keyword_dict_["feature"].begin(), keyword_dict_["feature"].end(), keyword) !=
            keyword_dict_["feature"].end()) {
            score += 0.5f;
        }
    }

    return std::min(1.0f, score);  // 最高1.0
}

// 计算创意性分数: 统计创意类关键词(旅游、娱乐等)
float QueryClassifier::calculate_creative_score(const std::vector<std::string> &keywords) {
    float score = 0.0f;

    // 每个创意关键词加0.3分
    for (const auto &keyword : keywords) {
        if (std::find(keyword_dict_["creative"].begin(), keyword_dict_["creative"].end(),
                      keyword) != keyword_dict_["creative"].end()) {
            score += 0.3f;
        }
    }

    return std::min(1.0f, score);  // 最高1.0
}

// 检测是否含有疑问词(什么、怎么、为什么等)
bool QueryClassifier::detect_question_words(const std::string &query) {
    for (const auto &word : keyword_dict_["question"]) {
        if (query.find(word) != std::string::npos) {
            return true;  // 找到疑问词
        }
    }
    return false;  // 没有疑问词
}

// 检测是否含有紧急词(故障、危险、警告等)
bool QueryClassifier::detect_emergency_words(const std::string &query) {
    for (const auto &word : keyword_dict_["emergency"]) {
        if (query.find(word) != std::string::npos) {
            return true;  // 找到紧急词
        }
    }
    return false;  // 没有紧急词
}

// 检测是否含有技术词(发动机、制动、变速箱等)
bool QueryClassifier::detect_technical_words(const std::string &query) {
    for (const auto &word : keyword_dict_["technical"]) {
        if (query.find(word) != std::string::npos) {
            return true;  // 找到技术词
        }
    }
    return false;  // 没有技术词
}

}  // namespace edge_llm_rag

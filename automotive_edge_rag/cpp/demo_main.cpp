#pragma once

#include <Python.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <chrono>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "ZmqClient.h"
#include "ZmqServer.h"
#include "edge_llm_rag_system.h"

using namespace edge_llm_rag;

// 全局ZMQ服务器对象,用于接收语音识别(ASR)发送的文本
zmq_component::ZmqServer server;

// 处理单个查询的函数
void process_query(EdgeLLMRAGSystem &system, const std::string &query) {
    std::cout << "\n 处理查询: " << query << std::endl;

    // 记录开始时间
    auto start_time = std::chrono::high_resolution_clock::now();

    // 调用RAG系统处理查询
    std::string response = system.process_query(query);

    // 记录结束时间
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // 打印结果
    std::cout << "\n系统响应:" << std::endl;
    std::cout << response << std::endl;
    std::cout << "\n响应时间: " << duration.count() << "ms" << std::endl;
}

// 退出信号处理函数(Ctrl+C时调用)
void exit_handler(int signal) {
    { std::cout << "程序即将退出" << std::endl; }

    exit(signal);
}

// 接收ASR数据并处理的主循环
void receive_asr_data_and_process(EdgeLLMRAGSystem &system) {
    while (true)  // 无限循环
    {
        std::string input_str;

        // 从ZMQ服务器接收语音识别结果
        input_str = server.receive();
        std::cout << "[voice -> RAG] received: " << input_str << std::endl;

        // 回复确认消息
        server.send("RAG success reply !!!");

        // 处理接收到的查询
        process_query(system, input_str);
    }
}

// 主函数: 系统入口
int main() {
    // 注册Ctrl+C信号处理函数
    signal(SIGINT, exit_handler);

    // 创建并初始化RAG系统
    std::cout << "初始化车载边缘LLM+RAG系统..." << std::endl;
    EdgeLLMRAGSystem system;  // 构造时会加载Python模块和向量模型

    // 初始化查询分类器和缓存
    if (!system.initialize()) {
        std::cerr << "系统初始化失败" << std::endl;
        return 1;
    }

    std::cout << "系统初始化成功" << std::endl;

    // 进入主循环,持续接收并处理语音输入
    receive_asr_data_and_process(system);

    return 0;
}
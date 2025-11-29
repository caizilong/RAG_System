/**
 * persistent_search_cli.cpp - C++调用Python搜索器的命令行工具
 *
 * 功能:
 * 1. 使用pybind11嵌入Python解释器
 * 2. 加载Python向量搜索模块
 * 3. 提供命令行和交互式两种模式
 * 4. 测试C++与Python集成
 *
 * 编译: g++ -std=c++17 persistent_search_cli.cpp -lpython3 -lpybind11
 * 运行: ./persistent_search_cli "发动机故障"
 */
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
namespace fs = std::filesystem;

namespace py = pybind11;

// 命令行参数结构体
struct CliOptions {
    int top_k        = 2;              // 返回前几个搜索结果
    double threshold = 0.5;            // 相似度阈值(0-1)
    std::vector<std::string> queries;  // 查询列表
};

/**
 * 解析命令行参数
 *
 * 支持的参数:
 * --top_k N 或 -k N: 设置返回结果数
 * --threshold T 或 -t T: 设置相似度阈值
 * --help 或 -h: 显示帮助信息
 * 其他参数: 作为查询字符串
 *
 * @param argc 参数个数
 * @param argv 参数数组
 * @return CliOptions 解析后的参数
 */
static CliOptions parse_args(int argc, char** argv) {
    CliOptions opts;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // 解析top_k参数
        if ((arg == "--top_k" || arg == "-k") && i + 1 < argc) {
            opts.top_k = std::stoi(argv[++i]);
        }
        // 解析threshold参数
        else if ((arg == "--threshold" || arg == "-t") && i + 1 < argc) {
            opts.threshold = std::stod(argv[++i]);
        }
        // 显示帮助
        else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [--top_k N] [--threshold T] [query ...]\n";
            std::exit(0);
        }
        // 其他参数作为查询
        else {
            opts.queries.emplace_back(std::move(arg));
        }
    }
    return opts;
}

/**
 * 主函数 - C++调用Python向量搜索器
 *
 * 流程:
 * 1. 初始化Python解释器
 * 2. 加载Python向量搜索模块
 * 3. 加载文本向量化模型
 * 4. 执行搜索(命令行模式或交互模式)
 */
int main(int argc, char** argv) {
    // 步骤1: 初始化Python解释器(RAII自动管理生命周期)
    py::scoped_interpreter guard{};

    // 步骤2: 解析命令行参数
    auto opts = parse_args(argc, argv);

    try {
        // 步骤3: 配置Python路径,添加python目录
        py::module_ sys = py::module_::import("sys");
        py::list path   = sys.attr("path");
        path.append("python");  // 让Python能找到我们的模块

        // 步骤4: 导入Python向量搜索模块
        py::module_ mod = py::module_::import("vehicle_vector_search");
        // 创建Python搜索器对象
        py::object searcher = mod.attr("VehicleVectorSearch")("vector_db");

        // 步骤5: 加载文本向量化模型(只加载一次,提高效率)
        std::cout << "Loading model once..." << std::endl;
        auto load_t0 = std::chrono::high_resolution_clock::now();

        // 获取模型路径(../models)
        fs::path cpp_dir    = fs::absolute(__FILE__).parent_path();
        fs::path model_path = cpp_dir.parent_path() / "models";
        searcher.attr("load_model")(model_path.string());  // 调用Python的load_model方法

        auto load_t1   = std::chrono::high_resolution_clock::now();
        double load_ms = std::chrono::duration<double, std::milli>(load_t1 - load_t0).count();
        std::cout << "Model loaded (" << std::fixed << std::setprecision(2) << load_ms << " ms)"
                  << std::endl;

        // 步骤6: 打印数据库统计信息
        py::object stats = searcher.attr("get_statistics")();
        std::cout << "Stats: total_documents=" << stats["total_documents"].cast<int>()
                  << ", embedding_dimension=" << stats["embedding_dimension"].cast<int>()
                  << std::endl;

        // 步骤7: 定义搜索函数(lambda表达式)
        auto do_search = [&](const std::string& query) {
            // 计时开始
            auto t0 = std::chrono::high_resolution_clock::now();

            // 调用Python的search方法
            py::object results = searcher.attr("search")(query, opts.top_k, opts.threshold);

            // 计时结束
            auto t1   = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

            // 打印查询信息
            std::cout << "\nQuery: '" << query << "' (top_k=" << opts.top_k
                      << ", threshold=" << opts.threshold << ")\n";
            std::cout << "⏱  elapsed: " << std::fixed << std::setprecision(2) << ms << " ms\n";

            // 检查是否有结果
            if (py::len(results) == 0) {
                std::cout << "  No results" << std::endl;
                return;
            }

            // 遍历并打印搜索结果
            for (const auto& item : results) {
                double sim             = item["similarity"].cast<double>();       // 相似度
                std::string text       = item["text"].cast<std::string>();        // 文本内容
                std::string section    = item["section"].cast<std::string>();     // 章节
                std::string subsection = item["subsection"].cast<std::string>();  // 子章节

                std::cout << "  sim=" << std::fixed << std::setprecision(4) << sim
                          << ", section=" << section
                          << (subsection.empty() ? "" : ("/" + subsection))
                          << ", text=" << text.substr(0, 100) << "...\n";
            }
        };

        // 步骤8: 执行搜索(两种模式)
        if (!opts.queries.empty()) {
            // 模式1: 命令行模式 - 搜索所有提供的查询后退出
            for (const auto& q : opts.queries) do_search(q);
        } else {
            // 模式2: 交互模式 - 持续接收用户输入
            std::cout << "\nInteractive mode. Enter query (or 'quit' to exit).\n";
            std::string line;
            while (true) {
                std::cout << "> ";
                if (!std::getline(std::cin, line)) break;     // 读取失败或EOF
                if (line == "quit" || line == "exit") break;  // 退出命令
                if (line.empty()) continue;                   // 跳过空输入
                do_search(line);                              // 执行搜索
            }
        }

        return 0;
    } catch (const py::error_already_set& e) {
        // 捕获Python异常
        std::cerr << "Python异常: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        // 捕获C++异常
        std::cerr << "异常: " << e.what() << std::endl;
        return 1;
    }
}
#include <iostream>
#include <format>

#include <Windows.h>

#include "liboai.h"

import message;
import context;
import openai_client;

// 辅助函数：打印分隔线
void print_separator(const std::string& title)
{
	std::cout << "\n" << std::string(60, '=') << "\n";
	std::cout << "[测试] " << title << "\n";
	std::cout << std::string(60, '=') << "\n";
}

// 基础配置
nlohmann::json make_config()
{
	auto config{ nlohmann::json::object() };
	config["model"] = "moonshot-v1-8k";
	config["base_url"] = "https://api.moonshot.cn/v1";
	config["api_key"] = "sk-Pqgja7LiXQ9iG2D6HJpoLrkyiu7Pb7PThoPFjgJqFvg37y4B";
	return config;
}

int main()
{
	SetConsoleOutputCP(CP_UTF8);

	openai_client client{};
	auto config = make_config();

	// ─────────────────────────────────────────────
	// 测试 1：基本同步调用（只有 user message）
	// ─────────────────────────────────────────────
	try
	{
		print_separator("1. 基本同步调用");
		context_shared_ptr ctx{ std::make_shared<context>() };
		ctx->append(std::make_shared<message>(message::role::user, "你好，请简短介绍一下自己"));

		auto result = client.generate(config, ctx);
		std::cout << *result << "\n";
	}
	catch (const std::exception& e)
	{
		std::cerr << "❌ 失败: " << e.what() << "\n";
	}

	// ─────────────────────────────────────────────
	// 测试 2：带 system message 的同步调用
	// ─────────────────────────────────────────────
	try
	{
		print_separator("2. 带 system message");
		context_shared_ptr ctx{ std::make_shared<context>() };
		ctx->append(std::make_shared<message>(message::role::system, "你是一个暴躁的程序员，喜欢用简短的话回答问题。"));
		ctx->append(std::make_shared<message>(message::role::user, "什么是死锁？"));

		auto result = client.generate(config, ctx);
		std::cout << *result << "\n";
	}
	catch (const std::exception& e)
	{
		std::cerr << "❌ 失败: " << e.what() << "\n";
	}

	// ─────────────────────────────────────────────
	// 测试 3：带可选参数的调用
	// ─────────────────────────────────────────────
	try
	{
		print_separator("3. 带可选参数 (temperature, max_tokens, stop)");
		auto cfg = make_config();
		cfg["temperature"] = 0.2f;
		cfg["max_tokens"] = 50;
		cfg["stop"] = nlohmann::json::array({"。", "?"});

		context_shared_ptr ctx{ std::make_shared<context>() };
		ctx->append(std::make_shared<message>(message::role::user, "用一句话概括 C++ 的优点"));

		auto result = client.generate(cfg, ctx);
		std::cout << *result << "\n";
	}
	catch (const std::exception& e)
	{
		std::cerr << "❌ 失败: " << e.what() << "\n";
	}

	// ─────────────────────────────────────────────
	// 测试 4：流式回调（逐 token 输出）
	// ─────────────────────────────────────────────
	try
	{
		print_separator("4. 流式回调 (streaming)");
		context_shared_ptr ctx{ std::make_shared<context>() };
		ctx->append(std::make_shared<message>(message::role::user, "写一首四行诗"));

		std::cout << "助手: ";
		auto stream_cb = [](std::string data) -> bool {
			std::cout << data << std::flush;
			return true;  // 继续接收
		};

		auto result = client.generate(config, ctx, stream_cb);
		std::cout << "\n\n完整结果:\n" << *result << "\n";
	}
	catch (const std::exception& e)
	{
		std::cerr << "❌ 失败: " << e.what() << "\n";
	}

	// ─────────────────────────────────────────────
	// 测试 5：异步调用
	// ─────────────────────────────────────────────
	try
	{
		print_separator("5. 异步调用 (async)");
		context_shared_ptr ctx{ std::make_shared<context>() };
		ctx->append(std::make_shared<message>(message::role::user, "1+1=几？"));

		std::cout << "发起异步请求...\n";
		auto future = client.generate_async(config, ctx);

		// 模拟做点别的事
		std::cout << "等待结果中...\n";
		auto result = future.get();
		std::cout << *result << "\n";
	}
	catch (const std::exception& e)
	{
		std::cerr << "❌ 失败: " << e.what() << "\n";
	}

	// ─────────────────────────────────────────────
	// 测试 6：异常 - 多个 system message
	// ─────────────────────────────────────────────
	try
	{
		print_separator("6. 异常测试：多个 system message（应抛异常）");
		context_shared_ptr ctx{ std::make_shared<context>() };
		ctx->append(std::make_shared<message>(message::role::system, "系统提示 1"));
		ctx->append(std::make_shared<message>(message::role::system, "系统提示 2"));
		ctx->append(std::make_shared<message>(message::role::user, "测试"));

		auto result = client.generate(config, ctx);
		std::cout << *result << "\n";
		std::cerr << "❌ 未抛异常，测试失败！\n";
	}
	catch (const std::runtime_error& e)
	{
		std::cout << "✅ 正确捕获异常: " << e.what() << "\n";
	}
	catch (const std::exception& e)
	{
		std::cerr << "⚠️  捕获非预期异常: " << e.what() << "\n";
	}

	// ─────────────────────────────────────────────
	// 测试 7：异常 - system 不在第一位
	// ─────────────────────────────────────────────
	try
	{
		print_separator("7. 异常测试：system 不在第一位（应抛异常）");
		context_shared_ptr ctx{ std::make_shared<context>() };
		ctx->append(std::make_shared<message>(message::role::user, "用户先问"));
		ctx->append(std::make_shared<message>(message::role::system, "系统提示在后"));

		auto result = client.generate(config, ctx);
		std::cout << *result << "\n";
		std::cerr << "❌ 未抛异常，测试失败！\n";
	}
	catch (const std::runtime_error& e)
	{
		std::cout << "✅ 正确捕获异常: " << e.what() << "\n";
	}
	catch (const std::exception& e)
	{
		std::cerr << "⚠️  捕获非预期异常: " << e.what() << "\n";
	}

	// ─────────────────────────────────────────────
	// 测试 8：多轮对话
	// ─────────────────────────────────────────────
	try
	{
		print_separator("8. 多轮对话");
		context_shared_ptr ctx{ std::make_shared<context>() };
		ctx->append(std::make_shared<message>(message::role::system, "你是一个数学助手。"));
		ctx->append(std::make_shared<message>(message::role::user, "2+3=？"));

		auto result1 = client.generate(config, ctx);
		std::cout << "第一轮:\n" << *result1 << "\n\n";

		// 把结果追加到 context 继续对话
		ctx->merge(result1);
		ctx->append(std::make_shared<message>(message::role::user, "再加 5 呢？"));

		auto result2 = client.generate(config, ctx);
		std::cout << "第二轮:\n" << *result2 << "\n";
	}
	catch (const std::exception& e)
	{
		std::cerr << "❌ 失败: " << e.what() << "\n";
	}

	// ─────────────────────────────────────────────
	// 汇总
	// ─────────────────────────────────────────────
	std::cout << "\n" << std::string(60, '=') << "\n";
	std::cout << "所有测试执行完毕\n";
	std::cout << std::string(60, '=') << "\n";

	return 0;
}

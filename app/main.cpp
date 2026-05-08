#include <iostream>
#include <format>

#include <Windows.h>

#include "nlohmann/json.hpp"

import context;
import message;
import llm_provider;
import provider_registry;
import openai_provider;

// 辅助函数：打印分隔线
void print_separator(const std::string& title)
{
	std::cout << "\n" << std::string(60, '=') << "\n";
	std::cout << "[测试] " << title << "\n";
	std::cout << std::string(60, '=') << "\n";
}

// 从环境变量读取 API key
std::string get_api_key_from_env()
{
	const char* key = std::getenv("CPPGENT_API_KEY");
	return key ? key : "";
}

int main()
{
	SetConsoleOutputCP(CP_UTF8);

	// 注册 OpenAI provider
	provider_registry::instance().register_factory(
		"openai",
		std::make_shared<openai_factory>()
	);

	// 创建 provider
	auto provider = provider_registry::instance().create("openai");
	if (!provider)
	{
		std::cerr << "无法创建 provider\n";
		return 1;
	}

	// 配置
	nlohmann::json config{};
	config["model"] = "moonshot-v1-8k";
	config["base_url"] = "https://api.moonshot.cn/v1";
	config["api_key"] = get_api_key_from_env();

	if (config["api_key"].get<std::string>().empty())
	{
		std::cerr << "CPPGENT_API_KEY 环境变量未设置\n";
		return 1;
	}

	provider->set_config(config);

	// 创建对话上下文
	auto ctx = std::make_shared<context>();
	ctx->append(std::make_shared<message>(message::role::system, "你是一个 helpful assistant"));
	ctx->append(std::make_shared<message>(message::role::user, "你好"));

	// 调用生成
	try
	{
		auto result = provider->generate(ctx);
		if (result.message)
		{
			std::cout << "Assistant: " << result.message->get_content() << "\n";
		}
		if (result.usage)
		{
			std::cout << "Tokens: " << result.usage->total_tokens << "\n";
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}

	return 0;
}

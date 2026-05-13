#include <memory>
#include <fstream>
#include <iostream>

#include <Windows.h>

#include "nlohmann/json.hpp"

import context;
import llm_api;
import message;
import mcp_client;
import mcp_manager;
import api_registry;
import chat_completion_api;

std::string load_api_key(const std::string& path)
{
	std::string key{};
	if (std::ifstream file{ path }; file.is_open())
	{
		std::getline(file, key);
	}
	return key;
}

nlohmann::json load_mcp_config()
{
	constexpr auto mcp_config_json = R"({
  "mcpServers": {
    "filesystem": {
      "command": "npx",
      "args": [ "-y", "@modelcontextprotocol/server-filesystem", "D:/" ]
    }
  }
})";
	return nlohmann::json::parse(mcp_config_json);
}

int main()
{
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	nlohmann::json config{};
	config["model"] = "kimi-k2.6";
	config["base_url"] = "https://api.moonshot.cn";
	config["api_key"] = load_api_key(R"(D:\Sources\cppagent\api_key.txt)");

	api_registry::instance().register_factory("openai", std::make_shared<chat_completion_api_factory>());
	auto provider{ api_registry::instance().create("openai") };
	provider->set_config(config);

	auto mcp{ std::make_shared<mcp_manager>() };
	mcp->load(load_mcp_config());

	auto ctx{ std::make_shared<context>() };
	ctx->set_instructions("you are a helpful assistant");

	while (true)
	{
		std::cout << "请输入提示词:";

		std::string promat{};
		std::getline(std::cin, promat);

		ctx->append(std::make_shared<user_message>(promat));

		model_response_shared_ptr resp{};
		do
		{
			resp = provider->generate_text(ctx, mcp->get_tools());
		} while (!(resp->finish_reason == "stop" || resp->finish_reason == "length"));

		std::cout << ctx->messages_ref().back()->get_content() << '\n' << std::endl;
	}

	return 0;
}

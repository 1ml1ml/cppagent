#include <fstream>
#include <iostream>

#include <Windows.h>

#include "ai/logger.h"
#include "nlohmann/json.hpp"

import agent;
import api_registry;
import chat_completion_api;
import context;
import message;
import mcp_manager;

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
  return nlohmann::json::parse(
    R"({ "mcpServers": {
    "filesystem": {
      "command": "npx",
      "args": [ "-y", "@modelcontextprotocol/server-filesystem", "D:/" ]
    }
  }
})");
}

int main()
{
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);

  std::static_pointer_cast<ai::logger::ConsoleLogger>(
    ai::logger::detail::logger_instance()
  )->set_min_level(ai::logger::LogLevel::kLogLevelInfo);

  // Register API factories
  api_registry::instance().register_factory(
    "openai", std::make_shared<chat_completion_api_factory>()
  );

  // Configure agent
  auto chat_agent{ std::make_shared<agent>() };

  nlohmann::json model_config{};
  model_config["api"] = "openai";
  model_config["model"] = "moonshot-v1-128k";
  model_config["base_url"] = "https://api.moonshot.cn";
  model_config["api_key"] = load_api_key(R"(D:\Sources\cppagent\api_key.txt)");
  chat_agent->set_model_config(model_config);

  auto mcp{ std::make_shared<mcp_manager>() };
  mcp->load(load_mcp_config());
  chat_agent->set_mcp_manager(mcp);

  // Chat context
  auto ctx{ std::make_shared<context>() };
  ctx->set_instructions("you are a helpful assistant");

  // Main loop
  while (true)
  {
    std::cout << "请输入提示词:";

    std::string prompt{};
    std::getline(std::cin, prompt);

    if (prompt == "/exit")
    {
      break;
    }

    ctx->append(std::make_shared<user_message>(prompt));

    chat_agent->generate_text(ctx);

    std::cout << ctx->messages_ref().back()->get_content() << '\n' << std::endl;
  }

  return 0;
}

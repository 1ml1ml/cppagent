#include <filesystem>
#include <fstream>
#include <iostream>

#include <Windows.h>

import agent;
import context;
import message;
import app_config;
import mcp_manager;
import api_registry;
import chat_completion_api;

int main()
{
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);

  app_config cfg{};
  cfg.load(R"(D:\Sources\cppagent\config.json)");

  api_registry::instance().register_factory(cfg.model_config()["api"].get<std::string>(), std::make_shared<chat_completion_api_factory>());

  auto mcp{ std::make_shared<mcp_manager>() };
  mcp->load(cfg.mcp_servers());

  auto chat_agent{ std::make_shared<agent>() };
  chat_agent->set_model_config(cfg.model_config());
  chat_agent->set_mcp_manager(mcp);

  auto ctx{ std::make_shared<context>() };
  ctx->set_instructions("你是一个辅助机器人");

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

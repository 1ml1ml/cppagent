#include <fstream>
#include <iostream>
#include <filesystem>

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

  api_registry::instance().register_factory("chat completion", std::make_shared<chat_completion_api_factory>());

  app_config cfg{};
  cfg.load(R"(D:\Sources\cppagent\config.json)");

  auto chat_agent{ std::make_shared<agent>() };
  chat_agent->set_model_config(cfg.model_config());

  try
  {
    auto mcp{ std::make_shared<mcp_manager>() };
    mcp->load(cfg.mcp_servers());
    chat_agent->set_mcp_manager(mcp);
  }
  catch (const std::exception& e)
  {
    std::cout << "mcp exception:" << e.what() << std::endl;
  }

  auto ctx{ std::make_shared<context>() };
  ctx->set_instructions("你是一个辅助机器人");

  while (true)
  {
    std::cout << "请输入提示词:" << std::endl;

    std::string prompt{};
    std::getline(std::cin, prompt);

    if (prompt == "/exit")
    {
      break;
    }

    ctx->append(std::make_shared<user_message>(prompt));

    try
    {
      auto resp{ chat_agent->generate_text(ctx) };
      std::cout << resp->message->get_content() << '\n' << std::endl;
    }
    catch (const std::exception& e)
    {
      std::cout << "generate text exception:" << e.what() << std::endl;
    }
  }

  return 0;
}

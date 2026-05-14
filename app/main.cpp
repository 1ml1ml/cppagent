#include <filesystem>
#include <fstream>
#include <iostream>

#include <Windows.h>

#include "ai/logger.h"

import app_config;
import agent;
import api_registry;
import chat_completion_api;
import context;
import message;
import mcp_manager;

int main()
{
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);

  std::static_pointer_cast<ai::logger::ConsoleLogger>(
    ai::logger::detail::logger_instance()
  )->set_min_level(ai::logger::LogLevel::kLogLevelInfo);

  // Load configuration
  app_config cfg{};
  cfg.load(std::filesystem::path(__FILE__).parent_path() / "config.json");

  // Register API factories
  api_registry::instance().register_factory(
    cfg.api_provider(), std::make_shared<chat_completion_api_factory>()
  );

  // Configure agent
  auto chat_agent{ std::make_shared<agent>() };

  nlohmann::json model_config{};
  model_config["api"] = cfg.api_provider();
  model_config["model"] = cfg.model();
  model_config["base_url"] = cfg.base_url();
  model_config["api_key"] = cfg.api_key();
  chat_agent->set_model_config(model_config);

  auto mcp{ std::make_shared<mcp_manager>() };
  mcp->load(cfg.mcp_servers());
  chat_agent->set_mcp_manager(mcp);

  // Chat context
  auto ctx{ std::make_shared<context>() };
  ctx->set_instructions(cfg.instructions());

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

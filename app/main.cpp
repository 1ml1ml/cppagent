#include <memory>
#include <fstream>
#include <iostream>
#include <filesystem>

#include <Windows.h>

#include "nlohmann/json.hpp"

import context;
import message;
import llm_provider;
import openai_provider;
import provider_registry;
import mcp_manager;
import mcp_client;

std::string load_api_key(const std::string& path)
{
  std::ifstream file{ path };
  if (!file.is_open())
  {
    return {};
  }

  std::string key{};
  std::getline(file, key);
  return key;
}

nlohmann::json load_mcp_config()
{
  std::ifstream file{ R"(D:\Sources\cppagent\mcp_config.json)" };
  if (!file.is_open())
  {
    return {};
  }

  nlohmann::json config{};
  file >> config;
  return config;
}

int main()
{
  SetConsoleOutputCP(CP_UTF8);

  nlohmann::json config{};
  config["model"] = "moonshot-v1-8k";
  config["base_url"] = "https://api.moonshot.cn/v1";
  config["api_key"] = load_api_key(R"(D:\Sources\cppagent\api_key.txt)");

  provider_registry::instance().register_factory( "openai", std::make_shared<openai_provider_factory>() );
  auto provider{ provider_registry::instance().create("openai") };
  provider->set_config(config);

  auto mcp{ std::make_shared<mcp_manager>() };
  mcp->load(load_mcp_config());

  auto ctx{ std::make_shared<context>() };
  ctx->set_instructions("you are a helpful assistant");
  ctx->append(std::make_shared<message>(message::role::user, "你好"));

  try
  {
    for (int i{ 0 }; i < 10; ++i)
    {
      auto result{ provider->generate(ctx, mcp->get_tools(), [](std::string) { return true; }) };

      if (result->finish_reason == "tool_call")
      {
        result->apply_to_context(ctx, mcp);
      }
      else
      {
        result->apply_to_context(ctx, mcp);
        break;
      }
    }

    std::cout << *ctx << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}

#include <memory>
#include <fstream>
#include <iostream>
#include <filesystem>

#include <Windows.h>

#include "nlohmann/json.hpp"

import context;
import message;
import mcp_client;
import mcp_manager;
import llm_provider;
import openai_provider;
import provider_registry;

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
	nlohmann::json config{};
  if (std::ifstream file{ R"(D:\Sources\cppagent\mcp_config.json)" }; file.is_open())
  {
    file >> config;
  }
	return config;
}

int main()
{
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);

  nlohmann::json config{};
  config["model"] = "moonshot-v1-8k";
  config["base_url"] = "https://api.moonshot.cn";
  config["api_key"] = load_api_key(R"(D:\Sources\cppagent\api_key.txt)");

  provider_registry::instance().register_factory( "openai", std::make_shared<openai_provider_factory>() );
  auto provider{ provider_registry::instance().create("openai") };
  provider->set_config(config);

  auto mcp{ std::make_shared<mcp_manager>() };
  mcp->load(load_mcp_config());

  auto ctx{ std::make_shared<context>() };
  ctx->set_instructions("you are a helpful assistant");

  while (true)
  {
    std::string promat{};
    std::getline(std::cin, promat);

    ctx->append(std::make_shared<message>(message::role::user, message::type::text, promat));
    
    model_response_shared_ptr resp{};
    do
    {
      resp = provider->generate(ctx, mcp->get_tools());
    } while (!(resp->finish_reason == "stop" || resp->finish_reason == "length"));

    std::cout.clear();
    std::cout << *ctx->messages_ref().back() << std::endl;
  }

  return 0;
}

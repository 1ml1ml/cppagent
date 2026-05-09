#include <iostream>

#include <Windows.h>

#include "nlohmann/json.hpp"

import context;
import message;
import llm_provider;
import openai_provider;
import provider_registry;

int main()
{
  SetConsoleOutputCP(CP_UTF8);

  nlohmann::json config{};
  config["model"] = "moonshot-v1-8k";
  config["base_url"] = "https://api.moonshot.cn/v1";
  config["api_key"] = "sk-pqXemyN0WzTTQ0GUSJm0BG8awWjtgpJZiWeTLovguC7NmXW2";

  provider_registry::instance().register_factory( "openai", std::make_shared<openai_factory>() );
  auto provider{ provider_registry::instance().create("openai") };
  provider->set_config(config);

  auto ctx{ std::make_shared<context>() };
  ctx->append(std::make_shared<message>(message::role::system, "you are a helpful assistant"));
  ctx->append(std::make_shared<message>(message::role::user, "你好"));

  try
  {
    auto result = provider->generate(ctx, [](std::string) { return true; });
    result->apply_to_context(ctx);
    
    std::cout << *ctx << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}

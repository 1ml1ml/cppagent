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

    if (auto* text = dynamic_cast<text_result*>(result.get()))
    {
      std::cout << "Assistant: " << text->message->get_content() << "\n";
      std::cout << "Tokens: " << text->usage.total_tokens << "\n";
      if (text->is_truncated())
      {
        std::cout << "[警告] 回复被截断\n";
      }
    }
    else if (auto* tool = dynamic_cast<tool_result*>(result.get()))
    {
      std::cout << "[工具调用] " << tool->tool_calls.size() << " 个工具待执行\n";
      for (const auto& call : tool->tool_calls)
      {
        std::cout << "  - " << call.function_name << "(" << call.arguments.dump() << ")\n";
      }
    }
    else
    {
      std::cerr << "未知结果类型\n";
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}

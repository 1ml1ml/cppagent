module;

#include <algorithm>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

#include "liboai.h"
#include "nlohmann/json.hpp"

module openai_provider;

import llm_provider;

import context;
import message;

class openai_provider::impl
{
public:
  static liboai::Conversation context_to_conversation(const context_shared_ptr& ctx);

public:
  nlohmann::json config{ nlohmann::json::object() };
};

liboai::Conversation openai_provider::impl::context_to_conversation(const context_shared_ptr& ctx)
{
  if (auto system_count { std::ranges::count_if(ctx->messages_ref(), [](const auto& msg) { return msg->get_role() == message::role::system; }) }; system_count > 1)
  {
    throw std::runtime_error("context contains multiple system messages (" + std::to_string(system_count) + "), expected at most 1");
  }
  else if (system_count == 1 && ctx->messages_ref().front()->get_role() != message::role::system)
  {
    throw std::runtime_error("context contains a system message that is not the first message");
  }

  nlohmann::json messages{ nlohmann::json::array() };
  for (const auto& msg : ctx->messages_ref())
  {
    messages.push_back(
    {
      {"role", message::role_to_string(msg->get_role())},
      {"content", std::string(msg->get_content())}
    });
  }

  nlohmann::json root{ nlohmann::json::object() };
  root["messages"] = messages;

  liboai::Conversation conv{};
  if (!conv.Import(root.dump()))
  {
    throw std::runtime_error("failed to convert context to conversation");
  }

  return conv;
}

openai_provider::openai_provider()
  : llm_provider(),
    impl{ std::make_unique<class impl>() }
{
}

openai_provider::~openai_provider() = default;

void openai_provider::set_config(const nlohmann::json& config)
{
  impl->config = config;
}

nlohmann::json openai_provider::get_config() const
{
  return impl->config;
}

generation_result_shared_ptr openai_provider::generate(const context_shared_ptr& ctx, const stream_callback& stream_callback)
{
  auto conv{ impl::context_to_conversation(ctx) };

  liboai::OpenAI oai{ impl->config["base_url"].get<std::string>() };
  oai.auth.SetKey(impl->config["api_key"].get<std::string>());

  auto response
  {
    oai.ChatCompletion->create(
      impl->config["model"].get<std::string>(),
      conv,
      impl->config.contains("function_call") ? std::optional{ impl->config["function_call"].get<std::string>() } : std::nullopt,
      impl->config.contains("temperature") ? std::optional{ impl->config["temperature"].get<float>() } : std::nullopt,
      impl->config.contains("top_p") ? std::optional{ impl->config["top_p"].get<float>() } : std::nullopt,
      impl->config.contains("n") ? std::optional{ impl->config["n"].get<std::uint16_t>() } : std::nullopt,
      [stream_callback](std::string data, intptr_t conversation_ptr, liboai::Conversation& conv)
      {
        conv.AppendStreamData(data);
        return stream_callback ? stream_callback(data) : true;
      },
      impl->config.contains("stop") ? std::optional{ impl->config["stop"].get<std::vector<std::string>>() } : std::nullopt,
      impl->config.contains("max_tokens") ? std::optional{ impl->config["max_tokens"].get<std::uint16_t>() } : std::nullopt,
      impl->config.contains("presence_penalty") ? std::optional{ impl->config["presence_penalty"].get<float>() } : std::nullopt,
      impl->config.contains("frequency_penalty") ? std::optional{ impl->config["frequency_penalty"].get<float>() } : std::nullopt,
      impl->config.contains("logit_bias") ? std::optional{ impl->config["logit_bias"].get<std::unordered_map<std::string, std::int8_t>>() } : std::nullopt,
      impl->config.contains("user") ? std::optional{ impl->config["user"].get<std::string>() } : std::nullopt)
  };

  return {};
}

std::future<generation_result_shared_ptr> openai_provider::generate_async(const context_shared_ptr& ctx, const stream_callback& stream_callback)
{
  return std::async(std::launch::async, [this, ctx, stream_callback]() { return generate(ctx, stream_callback); });
}

std::string_view openai_factory::name() const
{
  return "openai";
}

provider_unique_ptr openai_factory::create() const
{
  return std::make_unique<openai_provider>();
}

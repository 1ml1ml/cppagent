module;

#include <memory>
#include <future>
#include <string>
#include <vector>
#include <ranges>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <functional>

#include "liboai.h"
#include "nlohmann/json.hpp"

module openai_client;

import context;
import message;

class openai_client::impl
{
public:
	static std::shared_ptr<liboai::Conversation> context_to_conversation(const context_shared_ptr& ctx);
};

std::shared_ptr<liboai::Conversation> openai_client::impl::context_to_conversation(const context_shared_ptr& ctx)
{
	if (auto system_count{ std::ranges::count_if(ctx->messages(), [](const auto& msg) { return msg->get_role() == message::role::system; }) }; system_count > 1)
	{
		throw std::runtime_error("context contains multiple system messages (" + std::to_string(system_count) + "), expected at most 1");
	}
	else if (system_count == 1 && ctx->messages().front()->get_role() != message::role::system)
	{
		throw std::runtime_error("context contains a system message that is not the first message");
	}

	nlohmann::json messages{ nlohmann::json::array() };
	for (const auto& msg : ctx->messages())
	{
		messages.push_back({ {"role", message::role_to_string(msg->get_role())}, {"content", std::string(msg->get_content())} });
	}

	nlohmann::json root{ nlohmann::json::object() };
	root["messages"] = messages;

	auto conv{ std::make_shared<liboai::Conversation>() };
	conv->Import(root.dump());

	return conv;
}

openai_client::openai_client() :
	impl{ std::make_unique<class impl>() }
{
}

openai_client::~openai_client() = default;

context_shared_ptr openai_client::generate(const nlohmann::json& config, const context_shared_ptr& ctx, const std::function<bool(std::string data)>& stream_callback)
{
	auto conv{ impl::context_to_conversation(ctx) };

	liboai::OpenAI oai{ config["base_url"].get<std::string>() };
	oai.auth.SetKey(config["api_key"].get<std::string>());

	auto response{ oai.ChatCompletion->create(
	config["model"].get<std::string>(),
	*conv,
	config.contains("function_call") ? std::optional{config["function_call"].get<std::string>()} : std::nullopt,
	config.contains("temperature") ? std::optional{config["temperature"].get<float>()} : std::nullopt,
	config.contains("top_p") ? std::optional{config["top_p"].get<float>()} : std::nullopt,
	config.contains("n") ? std::optional{config["n"].get<std::uint16_t>()} : std::nullopt,
	[this, stream_callback](std::string data, intptr_t conversation_ptr, liboai::Conversation& conv) { conv.AppendStreamData(data); return stream_callback ? stream_callback(data) : true; },
	config.contains("stop") ? std::optional{config["stop"].get<std::vector<std::string>>()} : std::nullopt,
	config.contains("max_tokens") ? std::optional{config["max_tokens"].get<std::uint16_t>()} : std::nullopt,
	config.contains("presence_penalty") ? std::optional{config["presence_penalty"].get<float>()} : std::nullopt,
	config.contains("frequency_penalty") ? std::optional{config["frequency_penalty"].get<float>()} : std::nullopt,
	config.contains("logit_bias") ? std::optional{config["logit_bias"].get<std::unordered_map<std::string, std::int8_t>>()} : std::nullopt,
	config.contains("user") ? std::optional{config["user"].get<std::string>()} : std::nullopt) };

	auto result{ std::make_shared<context>() };
	result->append(std::make_shared<message>(message::role::assistant, conv->GetLastResponse()));
	return result;
}

std::future<context_shared_ptr> openai_client::generate_async(const nlohmann::json& config, const context_shared_ptr& ctx, const std::function<bool(std::string data)>& stream_callback)
{
	return std::async([this, config, ctx, stream_callback]() { return generate(config, ctx, stream_callback); });
}

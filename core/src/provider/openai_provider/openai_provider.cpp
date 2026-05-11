module;

#include <algorithm>
#include <functional>
#include <future>
#include <memory>
#include <optional>
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
	static std::vector<std::string> split_chunk(const std::string& chunk);

	static model_response_shared_ptr model_response_from_json(const nlohmann::json& response_json);

public:
	bool on_stream_callback(const std::string& data, intptr_t conversation_ptr, nlohmann::json& response_root, const stream_callback& callback);

public:
	nlohmann::json config{ nlohmann::json::object() };
};

liboai::Conversation openai_provider::impl::context_to_conversation(const context_shared_ptr& ctx)
{
	if (auto system_count{ std::ranges::count_if(ctx->messages_ref(), [](const auto& msg) { return msg->get_role() == message::role::system; }) }; system_count > 1)
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

std::vector<std::string> openai_provider::impl::split_chunk(const std::string& data)
{
	std::vector<std::string> lines{};

	std::string temp{};
	std::istringstream iss{ data };
	while (std::getline(iss, temp))
	{
		if (!temp.empty() && temp.back() == '\r')
		{
			temp.pop_back();
		}

		if (!temp.empty())
		{
			lines.push_back(std::move(temp));
		}
	}

	return lines;
}

model_response_shared_ptr openai_provider::impl::model_response_from_json(const nlohmann::json& response_json)
{
	model_response_shared_ptr result{};

	auto& choice{ response_json["choices"][0] };
	auto finish_reason{ choice["finish_reason"].get<std::string>() };

	if ( finish_reason == "tool_call")
	{
		auto tool_call_response{ std::make_shared<model_tool_call_response>() };
		result = tool_call_response;
	}
	else
	{
		result = std::make_shared<model_normal_response>();
	}

	result->finish_reason = finish_reason;
	result->id = response_json["id"].get<std::string>();
	result->model = response_json["model"].get<std::string>();

	auto& usage{ response_json["usage"] };
	result->usage.prompt_tokens = usage["prompt_tokens"].get<int>();
	result->usage.completion_tokens = usage["completion_tokens"].get<int>();
	result->usage.total_tokens = usage["total_tokens"].get<int>();

	auto& messages{ choice["messages"] };
	result->message = std::make_shared<message>(message::role::assistant, messages["content"].get<std::string>());

	return result;
}

bool openai_provider::impl::on_stream_callback(const std::string& data, intptr_t conversation_ptr, nlohmann::json& response_root, const stream_callback& callback)
{
	if (response_root.is_null())
	{
		response_root["choices"] = nlohmann::json::array();
	}

	for (const auto& chunk : impl::split_chunk(data))
	{
		if (chunk.starts_with("data:"))
		{
			if (chunk != "data: [DONE]")
			{
				auto chunk_json{ nlohmann::json::parse(chunk.substr(5)) };
				for (const auto& chunk_choice : chunk_json["choices"])
				{
					auto& response_choice{ response_root["choices"][chunk_choice["index"].get<std::size_t>()] };
					response_choice["index"] = chunk_choice["index"];

					auto& finish_reason{ chunk_choice["finish_reason"] };
					response_choice["finish_reason"] = finish_reason;

					if (!finish_reason.is_null())
					{
						response_root["usage"] = chunk_choice["usage"];

						response_root["id"] = chunk_json["id"];
						response_root["model"] = chunk_json["model"];
						response_root["object"] = chunk_json["object"];
						response_root["created"] = chunk_json["created"];
						response_root["system_fingerprint"] = chunk_json["system_fingerprint"];
						continue;
					}

					auto& delta{ chunk_choice["delta"] };
					auto& messages{ response_choice["messages"] };

					messages["content"] = (messages.contains("content") ? messages.value("content", "") : "") + delta["content"].get<std::string>();

					if (delta.contains("role"))
					{
						messages["role"] = delta["role"];
					}
				}
			}
		}
	}
	return callback(data);
}

openai_provider::openai_provider() : llm_provider(),
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

model_response_shared_ptr openai_provider::generate(const context_shared_ptr& ctx, const stream_callback& callback)
{
	auto conv{ impl::context_to_conversation(ctx) };

	liboai::OpenAI oai{ impl->config["base_url"].get<std::string>() };
	oai.auth.SetKey(impl->config["api_key"].get<std::string>());

	nlohmann::json response_root{};
	auto response
	{
		oai.ChatCompletion->create(
			impl->config["model"].get<std::string>(),
			conv,
			impl->config.contains("function_call") ? std::optional{ impl->config["function_call"].get<std::string>() } : std::nullopt,
			impl->config.contains("temperature") ? std::optional{ impl->config["temperature"].get<float>() } : std::nullopt,
			impl->config.contains("top_p") ? std::optional{ impl->config["top_p"].get<float>() } : std::nullopt,
			impl->config.contains("n") ? std::optional{ impl->config["n"].get<std::uint16_t>() } : std::nullopt,
			callback ?
			std::optional{[this, callback, &response_root](std::string data, intptr_t conversation_ptr, liboai::Conversation& conv)
			{
				return impl->on_stream_callback(data, conversation_ptr, response_root, callback);
			}} : std::nullopt,
			impl->config.contains("stop") ? std::optional{ impl->config["stop"].get<std::vector<std::string>>() } : std::nullopt,
			impl->config.contains("max_tokens") ? std::optional{ impl->config["max_tokens"].get<std::uint16_t>() } : std::nullopt,
			impl->config.contains("presence_penalty") ? std::optional{ impl->config["presence_penalty"].get<float>() } : std::nullopt,
			impl->config.contains("frequency_penalty") ? std::optional{ impl->config["frequency_penalty"].get<float>() } : std::nullopt,
			impl->config.contains("logit_bias") ? std::optional{ impl->config["logit_bias"].get<std::unordered_map<std::string, std::int8_t>>() } : std::nullopt,
			impl->config.contains("user") ? std::optional{ impl->config["user"].get<std::string>() } : std::nullopt)
	};

	return impl::model_response_from_json(callback ? response_root : response.raw_json);
}

std::future<model_response_shared_ptr> openai_provider::generate_async(const context_shared_ptr& ctx, const stream_callback& stream_callback)
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

module;

#include <future>
#include <memory>
#include <ranges>
#include <string>
#include <vector>
#include <sstream>
#include <optional>
#include <algorithm>
#include <stdexcept>
#include <functional>

#include <windows.h>
#include <wincrypt.h>

#include "liboai.h"
#include "nlohmann/json.hpp"

module openai_responses_provider;

import llm_provider;

import context;
import message;
import mcp_client;

class openai_responses_provider::impl
{
public:
	static std::vector<std::string> split_chunk(const std::string& chunk);

	static nlohmann::json make_tools_json(const std::vector<tool_info>& tools);
	static nlohmann::json make_input_json(const context_shared_ptr& ctx, liboai::OpenAI& oai);

	static model_response_shared_ptr make_model_response(const nlohmann::json& resp_json);

public:
	nlohmann::json config{ nlohmann::json::object() };
};

std::vector<std::string> openai_responses_provider::impl::split_chunk(const std::string& chunk)
{
	std::vector<std::string> lines{};

	std::string temp{};
	std::istringstream iss{ chunk };
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

nlohmann::json openai_responses_provider::impl::make_tools_json(const std::vector<tool_info>& tools)
{
	nlohmann::json tools_json{ nlohmann::json::array() };
	for (const auto& tool : tools)
	{
		tools_json.push_back(
			{
				{"type", "function"},
				{"function",
					{
						{"name", tool.name},
						{"description", tool.description},
						{"parameters", tool.input_schema}
					}
				}
			});
	}
	return tools_json;
}

nlohmann::json openai_responses_provider::impl::make_input_json(const context_shared_ptr& ctx, liboai::OpenAI& oai)
{
	auto input_json{ nlohmann::json::array() };
	for (const auto& msg : ctx->messages())
	{
		auto msg_json{ nlohmann::json::object() };
		msg_json["role"] = message::role_to_string(msg->get_role());

		auto content_json{ nlohmann::json::array() };
		content_json.push_back(
			{
				{"type", "input_text"},
				{"text", msg->get_content()}
			});

		for (auto& att : msg->attachments_ref())
		{
			if (att.file_id.empty())
			{
				auto resp{ oai.File->create(att.name, att.mime_type) };
				att.file_id = resp["file_id"].get<std::string>();
			}

			content_json.push_back(
				{
					{"type", "input_file"},
					{"filename", att.name},
					{"file_id", att.file_id}
				});

		}
		msg_json["content"] = content_json;
		input_json.push_back(msg_json);
	}
	return input_json;
}

model_response_shared_ptr openai_responses_provider::impl::make_model_response(const nlohmann::json& resp_json)
{
	return {};
}

openai_responses_provider::openai_responses_provider() : llm_provider(),
impl{ std::make_unique<class impl>() }
{
}

openai_responses_provider::~openai_responses_provider() = default;

void openai_responses_provider::set_config(const nlohmann::json& config)
{
	impl->config = config;
}

nlohmann::json openai_responses_provider::get_config() const
{
	return impl->config;
}

model_response_shared_ptr openai_responses_provider::generate(const context_shared_ptr& ctx, const std::vector<tool_info>& tools, const stream_callback& callback)
{
	liboai::OpenAI oai{ impl->config["base_url"].get<std::string>() };
	oai.auth.SetKey(impl->config["api_key"].get<std::string>());

	auto resp
	{
		oai.Responses->create(
			impl->config["model"].get<std::string>(),
			impl::make_input_json(ctx, oai),
			std::optional<std::string>{ctx->get_instructions()},
			impl->config.contains("reasoning") ? std::optional{impl->config["reasoning"]} : std::nullopt,
			impl->config.contains("text") ? std::optional{impl->config["text"]} : std::nullopt,
			impl->config.contains("max_output_tokens") ? std::optional{impl->config["max_output_tokens"].get<uint32_t>()} : std::nullopt,
			impl->config.contains("temperature") ? std::optional{impl->config["temperature"].get<float>()} : std::nullopt,
			impl->config.contains("top_p") ? std::optional{impl->config["top_p"].get<float>()} : std::nullopt,
			impl->config.contains("seed") ? std::optional{impl->config["seed"].get<uint32_t>()} : std::nullopt,
			impl->config.contains("tools") ? std::optional{impl->config["tools"].get<nlohmann::json>()} : std::nullopt,
			impl->config.contains("tool_choice") ? std::optional{impl->config["tool_choice"].get<nlohmann::json>()} : std::nullopt,
			impl->config.contains("parallel_tool_calls") ? std::optional{impl->config["parallel_tool_calls"].get<bool>()} : std::nullopt,
			impl->config.contains("store") ? std::optional{impl->config["store"].get<bool>()} : std::nullopt,
			impl->config.contains("previous_response_id") ? std::optional{impl->config["previous_response_id"].get<std::string>()} : std::nullopt,
			impl->config.contains("include") ? std::optional{impl->config["include"].get<nlohmann::json>()} : std::nullopt,
			impl->config.contains("metadata") ? std::optional{impl->config["metadata"].get<nlohmann::json>()} : std::nullopt,
			impl->config.contains("user") ? std::optional{impl->config["user"].get<std::string>()} : std::nullopt,
			impl->config.contains("truncation") ? std::optional{impl->config["truncation"].get<std::string>()} : std::nullopt,
			callback ? std::optional{[this, callback](std::string data, intptr_t) { return callback(data); }} : std::nullopt
		)
	};

	return impl::make_model_response(resp.raw_json);
}

std::future<model_response_shared_ptr> openai_responses_provider::generate_async(const context_shared_ptr& ctx, const std::vector<tool_info>& tools, const stream_callback& stream_callback)
{
	return std::async(std::launch::async, [this, ctx, tools, stream_callback]() { return generate(ctx, tools, stream_callback); });
}

std::string_view openai_responses_provider_factory::name() const
{
	return "openai_responses";
}

provider_unique_ptr openai_responses_provider_factory::create() const
{
	return std::make_unique<openai_responses_provider>();
}

module;

#include <future>
#include <memory>
#include <ranges>
#include <string>
#include <vector>
#include <optional>
#include <algorithm>
#include <stdexcept>
#include <functional>

#include <windows.h>
#include <wincrypt.h>

#include "liboai.h"
#include "nlohmann/json.hpp"

module openai_provider;

import llm_provider;

import context;
import message;
import mcp_client;

class openai_provider::impl
{
public:
	static std::vector<std::string> split_chunk(const std::string& chunk);
	static liboai::Conversation make_conversation(const context_shared_ptr& ctx, const std::vector<tool_info>& tools);
	static model_response_shared_ptr model_response_from_json(const nlohmann::json& resp_json);

	static bool is_text_mime_type(const std::string& mime_type);
	static std::string base64_encode(const std::vector<std::byte>& data);

public:
	bool on_stream_callback(const std::string& data, intptr_t conversation_ptr, nlohmann::json& resp_json, const stream_callback& callback);

public:
	nlohmann::json config{ nlohmann::json::object() };
};

bool openai_provider::impl::is_text_mime_type(const std::string& mime_type)
{
	static const std::vector<std::string> text_types
	{
		"application/json",
		"application/xml",
		"application/javascript",
		"application/x-javascript",
		"application/sql",
		"application/graphql",
		"application/yaml",
		"application/x-yaml",
		"application/toml",
		"application/x-sh",
		"application/x-csh",
		"application/x-tcl",
		"application/x-perl",
		"application/x-ruby",
		"application/x-httpd-php",
		"application/x-lua",
		"application/x-python-code",
		"application/json-lines",
		"application/x-json",
	};
	return mime_type.starts_with("text/") || std::ranges::find(text_types, mime_type) != text_types.end();
}

std::string openai_provider::impl::base64_encode(const std::vector<std::byte>& data)
{
	if (data.empty())
	{
		return {};
	}

	DWORD size{};
	CryptBinaryToStringA(reinterpret_cast<const BYTE*>(data.data()), static_cast<DWORD>(data.size()), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &size);

	std::string result(size - 1, '\0');
	CryptBinaryToStringA(reinterpret_cast<const BYTE*>(data.data()), static_cast<DWORD>(data.size()), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, result.data(), &size);

	return result;
}

std::vector<std::string> openai_provider::impl::split_chunk(const std::string& chunk)
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

liboai::Conversation openai_provider::impl::make_conversation(const context_shared_ptr& ctx, const std::vector<tool_info>& tools)
{
	nlohmann::json messages_json{ nlohmann::json::array() };
	for (const auto& msg : ctx->messages())
	{
		auto msg_json{ nlohmann::json::object() };

		auto role{ msg->get_role() };
		std::string content{ msg->get_content() };

		if (role == message::role::tool)
		{
			auto tool_result_json{ nlohmann::json::parse(content) };
			msg_json["tool_call_id"] = tool_result_json["tool_call_id"];
			content = tool_result_json["content"].get<std::string>();
		}

		for (const auto& att : msg->attachments_ref())
		{
			if (is_text_mime_type(att.mime_type))
			{
				content += "\n[file: " + att.name + "]\n";
				content += "```\n";
				content += std::string(reinterpret_cast<const char*>(att.data.data()), att.data.size());
				content += "\n```";
			}
			else if (att.mime_type.starts_with("image/"))
			{
				content += "\n[image: " + att.name + "]\n";
				content += "data:" + att.mime_type + ";base64,";
				content += base64_encode(att.data);
			}
			else
			{
				throw std::runtime_error("unsupported attachment type: " + att.mime_type + " (file: " + att.name + ")");
			}
		}

		msg_json["role"] = message::role_to_string(role);
		msg_json["content"] = content;

		messages_json.push_back(msg_json);
	}

	nlohmann::json root{ nlohmann::json::object() };
	root["messages"] = messages_json;

	liboai::Conversation conv{};
	if (!conv.Import(root.dump()))
	{
		throw std::runtime_error("failed to convert context to conversation");
	}

	liboai::Functions functions{};
	for (const auto& tool : tools)
	{
		functions.AddFunction(tool.name);
		functions.SetDescription(tool.name, tool.description);

		if (tool.input_schema.contains("properties"))
		{
			std::vector<liboai::Functions::FunctionParameter> params{};
			for (const auto& [param_name, param_schema] : tool.input_schema["properties"].items())
			{
				params.emplace_back(param_name,
					param_schema["type"].get<std::string>(),
					param_schema.value("description", ""),
					param_schema.contains("enum") ? std::optional{ param_schema["enum"].get<std::vector<std::string>>() } : std::nullopt);
			}

			if (!params.empty())
			{
				functions.AppendParameters(tool.name, params);
			}
		}

		if (tool.input_schema.contains("required"))
		{
			functions.SetRequired(tool.name, tool.input_schema["required"].get<std::vector<std::string>>());
		}
	}
	conv.SetFunctions(functions);

	return conv;
}

model_response_shared_ptr openai_provider::impl::model_response_from_json(const nlohmann::json& resp_json)
{
	model_response_shared_ptr resp{};

	auto& choice{ resp_json["choices"][0] };
	auto finish_reason{ choice["finish_reason"].get<std::string>() };

	if (finish_reason == "tool_call")
	{
		auto tool_call_response{ std::make_shared<model_tool_call_response>() };
		resp = tool_call_response;
	}
	else
	{
		resp = std::make_shared<model_normal_response>();
	}

	resp->finish_reason = finish_reason;
	resp->id = resp_json["id"].get<std::string>();
	resp->model = resp_json["model"].get<std::string>();

	auto& usage{ resp_json["usage"] };
	resp->usage.prompt_tokens = usage["prompt_tokens"].get<int>();
	resp->usage.completion_tokens = usage["completion_tokens"].get<int>();
	resp->usage.total_tokens = usage["total_tokens"].get<int>();

	auto& messages{ choice["messages"] };
	resp->message = std::make_shared<message>(message::role::assistant, messages["content"].get<std::string>());

	return resp;
}

bool openai_provider::impl::on_stream_callback(const std::string& data, intptr_t, nlohmann::json& resp_json, const stream_callback& callback)
{
	if (resp_json.is_null())
	{
		resp_json["choices"] = nlohmann::json::array();
	}

	for (const auto& chunk : impl::split_chunk(data))
	{
		if (chunk.starts_with("data:"))
		{
			if (chunk == "data: [DONE]")
			{
				break;
			}

			auto chunk_json{ nlohmann::json::parse(chunk.substr(5)) };
			for (const auto& chunk_choice : chunk_json["choices"])
			{
				auto& resp_choice{ resp_json["choices"][chunk_choice["index"].get<std::size_t>()] };
				resp_choice["index"] = chunk_choice["index"];

				auto& finish_reason{ chunk_choice["finish_reason"] };
				resp_choice["finish_reason"] = finish_reason;

				if (!finish_reason.is_null())
				{
					resp_json["usage"] = chunk_choice["usage"];

					resp_json["id"] = chunk_json["id"];
					resp_json["model"] = chunk_json["model"];
					resp_json["object"] = chunk_json["object"];
					resp_json["created"] = chunk_json["created"];
					resp_json["system_fingerprint"] = chunk_json["system_fingerprint"];
					continue;
				}

				auto& delta{ chunk_choice["delta"] };
				auto& messages{ resp_choice["messages"] };

				messages["content"] = (messages.contains("content") ? messages.value("content", "") : "") + delta["content"].get<std::string>();

				if (delta.contains("role"))
				{
					messages["role"] = delta["role"];
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

model_response_shared_ptr openai_provider::generate(const context_shared_ptr& ctx, const std::vector<tool_info>& tools, const stream_callback& callback)
{
	liboai::OpenAI oai{ impl->config["base_url"].get<std::string>() };
	oai.auth.SetKey(impl->config["api_key"].get<std::string>());

	nlohmann::json resp_json{};
	auto conv{ impl::make_conversation(ctx, tools) };
	auto reqp
	{
		oai.ChatCompletion->create(
			impl->config["model"].get<std::string>(),
			conv,
			impl->config.contains("function_call") ? std::optional{ impl->config["function_call"].get<std::string>() } : std::nullopt,
			impl->config.contains("temperature") ? std::optional{ impl->config["temperature"].get<float>() } : std::nullopt,
			impl->config.contains("top_p") ? std::optional{ impl->config["top_p"].get<float>() } : std::nullopt,
			impl->config.contains("n") ? std::optional{ impl->config["n"].get<std::uint16_t>() } : std::nullopt,
			callback ?
			std::optional{[this, callback, &resp_json](std::string data, intptr_t conversation_ptr, liboai::Conversation& conv)
			{
				return impl->on_stream_callback(data, conversation_ptr, resp_json, callback);
			}} : std::nullopt,
			impl->config.contains("stop") ? std::optional{ impl->config["stop"].get<std::vector<std::string>>() } : std::nullopt,
			impl->config.contains("max_tokens") ? std::optional{ impl->config["max_tokens"].get<std::uint16_t>() } : std::nullopt,
			impl->config.contains("presence_penalty") ? std::optional{ impl->config["presence_penalty"].get<float>() } : std::nullopt,
			impl->config.contains("frequency_penalty") ? std::optional{ impl->config["frequency_penalty"].get<float>() } : std::nullopt,
			impl->config.contains("logit_bias") ? std::optional{ impl->config["logit_bias"].get<std::unordered_map<std::string, std::int8_t>>() } : std::nullopt,
			impl->config.contains("user") ? std::optional{ impl->config["user"].get<std::string>() } : std::nullopt)
	};

	return impl::model_response_from_json(callback ? resp_json : reqp.raw_json);
}

std::future<model_response_shared_ptr> openai_provider::generate_async(const context_shared_ptr& ctx, const std::vector<tool_info>& tools, const stream_callback& stream_callback)
{
	return std::async(std::launch::async, [this, ctx, tools, stream_callback]() { return generate(ctx, tools, stream_callback); });
}

std::string_view openai_provider_factory::name() const
{
	return "openai";
}

provider_unique_ptr openai_provider_factory::create() const
{
	return std::make_unique<openai_provider>();
}

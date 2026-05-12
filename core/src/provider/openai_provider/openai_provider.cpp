module;

#include <future>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include <functional>

#include "ai/openai.h"
#include "ai/tools.h"
#include "nlohmann/json.hpp"

module openai_provider;

import llm_provider;

import context;
import message;
import mcp_client;

class openai_provider::impl
{
public:
	nlohmann::json config{ nlohmann::json::object() };

	ai::GenerateOptions make_go(const context_shared_ptr& ctx, const std::vector<tool_info>& tools);
};

ai::GenerateOptions openai_provider::impl::make_go(const context_shared_ptr& ctx, const std::vector<tool_info>& tools)
{
	ai::GenerateOptions go{};
	go.model = config["model"].get<std::string>();

	go.on_tool_call_start = [ctx](const ai::ToolCall& tool_call)
		{
			auto tool_call_json{ nlohmann::json::object() };
			tool_call_json["tool_call_id"] = tool_call.id;
			tool_call_json["tool_name"] = tool_call.tool_name;
			tool_call_json["arguments"] = tool_call.arguments;
			ctx->append(std::make_shared<message>(message::role::assistant, message::type::tool_call, tool_call_json.dump()));
		};

	go.on_tool_call_finish = [ctx](const ai::ToolResult& tool_result)
		{
			auto tool_result_json{ nlohmann::json::object() };
			tool_result_json["result"] = tool_result.result;
			tool_result_json["error"] = tool_result.error.has_value();
			tool_result_json["tool_call_id"] = tool_result.tool_call_id;
			ctx->append(std::make_shared<message>(message::role::user, message::type::tool_result, tool_result_json.dump()));
		};

	go.messages.push_back(ai::Message::system(ctx->get_instructions().data()));
	for (const auto& msg : ctx->messages_ref())
	{
		switch (msg->get_role())
		{
		case message::role::user:
		{
			switch (msg->get_type())
			{
			case message::type::text: go.messages.push_back(ai::Message::user(msg->get_content().data())); break;

			case message::type::tool_result:
			{
				std::vector<ai::ToolResultContentPart> tool_results{};
				try {
					auto tool_result_json{ nlohmann::json::parse(msg->get_content()) };
					tool_results.emplace_back(tool_result_json["tool_call_id"].get<std::string>(), tool_result_json["result"], tool_result_json["error"].get<bool>());
				} catch (const std::exception& e) {
					go.messages.push_back(ai::Message::user(std::string(msg->get_content())));
					break;
				}
				go.messages.push_back(ai::Message::tool_results(tool_results));
				break;
			}

			default: throw std::runtime_error{ "Message role and type do not match" };
			}
			break;
		}

		case message::role::assistant:
		{
			switch (msg->get_type())
			{
			case message::type::text: go.messages.push_back(ai::Message::assistant(msg->get_content().data())); break;

			case message::type::tool_call:
			{
				std::vector<ai::ToolCallContentPart> tool_calls{};
				try {
					auto tool_call_json{ nlohmann::json::parse(msg->get_content()) };
					tool_calls.emplace_back(tool_call_json["tool_call_id"].get<std::string>(), tool_call_json["tool_name"].get<std::string>(), tool_call_json["arguments"]);
				} catch (const std::exception& e) {
					go.messages.push_back(ai::Message::assistant(std::string(msg->get_content())));
					break;
				}
				go.messages.push_back(ai::Message::assistant_with_tools("", tool_calls));
				break;
			}

			default: throw std::runtime_error{ "Message role and type do not match" };
			}
			break;
		}
		}
	}

	for (const auto& tool : tools)
	{
		go.tools.insert(std::pair{ tool.name, ai::Tool{ tool.description, tool.input_schema, [&ctx, &tool](const ai::JsonValue& args, const ai::ToolExecutionContext& context) -> ai::JsonValue
			{
				auto result{ tool.exec(args) };
				if (result.is_error)
				{
					throw std::runtime_error{ result.content };
				}
				return result.content;
			} } });
	}

	return go;
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
	auto result{ ai::openai::create_client(impl->config["api_key"].get<std::string>(), impl->config["base_url"].get<std::string>()).generate_text(impl->make_go(ctx, tools)) };
	if (!result.is_success())
	{
		throw std::runtime_error{ result.error_message() };
	}

	auto resp{ std::make_shared<model_response>() };
	resp->id = result.id.value();
	resp->model = result.model.value();
	resp->finish_reason = result.finishReasonToString();
	resp->usage.total_tokens = result.usage.total_tokens;
	resp->usage.prompt_tokens = result.usage.prompt_tokens;
	resp->usage.completion_tokens = result.usage.completion_tokens;

	if (result.finish_reason == ai::FinishReason::kFinishReasonStop)
	{
		resp->message = std::make_shared<message>(message::role::assistant, message::type::text, result.text);
		ctx->append(std::make_shared<message>(message::role::assistant, message::type::text, result.text));
	}

	return resp;
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

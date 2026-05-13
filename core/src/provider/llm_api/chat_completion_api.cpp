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

module chat_completion_api;

import llm_provider;

import context;
import message;
import mcp_client;

class message_to_go : public message_visitor
{
public:
	message_to_go(ai::GenerateOptions& go);

public:
	void visit_user_message(const std::shared_ptr<user_message>& message) override;
	void visit_assistant_message(const std::shared_ptr<assistant_message>& message) override;

public:
	ai::GenerateOptions& go;
};

message_to_go::message_to_go(ai::GenerateOptions& go) : message_visitor(),
go{ go }
{
}

void message_to_go::visit_user_message(const std::shared_ptr<user_message>& message)
{
	if (message->get_tool_call_results_ref().size())
	{
		if (message->get_content().size())
		{
			throw std::runtime_error{ "Role messages are not allowed to have both tool_call and content at the same time" };
		}

		std::vector<ai::ToolResultContentPart> tool_call_results{};
		for (const auto& tool_call_result : message->get_tool_call_results_ref())
		{
			tool_call_results.emplace_back(tool_call_result.tool_call_id, tool_call_result.result, tool_call_result.error);
		}
		go.messages.push_back(ai::Message::tool_results(tool_call_results));
	}
	else
	{
		go.messages.push_back(ai::Message::user(message->get_content().data()));
	}
}

void message_to_go::visit_assistant_message(const std::shared_ptr<assistant_message>& message)
{
	if (message->get_tool_calls_ref().size())
	{
		std::vector<ai::ToolCallContentPart> tool_calls{};
		for (const auto& tool_call : message->get_tool_calls_ref())
		{
			tool_calls.emplace_back(tool_call.id, tool_call.tool_name, tool_call.arguments);
		}
		go.messages.push_back(ai::Message::assistant_with_tools(message->get_content().data(), tool_calls));
	}
	else
	{
		go.messages.push_back(ai::Message::assistant(message->get_content().data()));
	}
}

class chat_completion_api::impl
{
public:
	nlohmann::json config{ nlohmann::json::object() };

	ai::GenerateOptions make_go(const context_shared_ptr& ctx, const std::vector<tool_info>& tools);
};

ai::GenerateOptions chat_completion_api::impl::make_go(const context_shared_ptr& ctx, const std::vector<tool_info>& tools)
{
	ai::GenerateOptions go{};
	go.model = config["model"].get<std::string>();

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

	go.system = ctx->get_instructions();

	auto convertor{ std::make_shared<message_to_go>(go) };
	for (const auto& message : ctx->messages_ref())
	{
		message->accept(convertor);
	}

	return go;
}

chat_completion_api::chat_completion_api() : llm_provider(),
impl{ std::make_unique<class impl>() }
{
}

chat_completion_api::~chat_completion_api() = default;

void chat_completion_api::set_config(const nlohmann::json& config)
{
	impl->config = config;
}

nlohmann::json chat_completion_api::get_config() const
{
	return impl->config;
}

model_response_shared_ptr chat_completion_api::generate_text(const context_shared_ptr& ctx, const std::vector<tool_info>& tools)
{
	auto go{ impl->make_go(ctx, tools) };

	auto result{ ai::openai::create_client(impl->config["api_key"].get<std::string>(), impl->config["base_url"].get<std::string>()).generate_text(go) };
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
		ctx->append(std::make_shared<assistant_message>(result.text));
		resp->message = std::make_shared<assistant_message>(result.text);
	}
	else if (result.finish_reason == ai::FinishReason::kFinishReasonToolCalls)
	{
		auto assistant_message{ std::make_shared<class assistant_message>(result.text) };

		std::vector<tool_call> tool_calls{};
		for (const auto& tool_call : result.tool_calls)
		{
			tool_calls.emplace_back(tool_call.id, tool_call.tool_name, tool_call.arguments);
		}
		assistant_message->set_tool_calls(tool_calls);
		ctx->append(assistant_message);

		auto user_message{ std::make_shared<class user_message>() };
		
		std::vector<tool_call_result> tool_results{};
		for (const auto& tool_result : result.tool_results)
		{
			tool_results.emplace_back(tool_result.tool_call_id, tool_result.error.has_value(), tool_result.result);
		}
		user_message->set_tool_call_results(tool_results);
		ctx->append(user_message);
	}

	return resp;
}

std::string_view chat_completion_api_factory::name() const
{
	return "openai";
}

provider_unique_ptr chat_completion_api_factory::create() const
{
	return std::make_unique<chat_completion_api>();
}

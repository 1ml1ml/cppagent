module;

#include <memory>
#include <string>
#include <vector>
#include <stdexcept>

#include "ai/tools.h"
#include "ai/openai.h"
#include "nlohmann/json.hpp"

module chat_completion_api;

import context;
import llm_api;
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

ai::GenerateOptions make_go(const nlohmann::json& config, const context_shared_ptr& ctx, const std::vector<tool_info>& tools)
{
	ai::GenerateOptions go{};
	go.model = config["model"].get<std::string>();

	for (const auto& tool : tools)
	{
		go.tools.insert(std::pair{ tool.name, ai::Tool{tool.description, tool.input_schema} });
	}

	go.system = ctx->get_instructions();

	auto convertor{ std::make_shared<message_to_go>(go) };
	for (const auto& message : ctx->messages_ref())
	{
		message->accept(convertor);
	}

	return go;
}

model_response_shared_ptr chat_completion_api::generate_text(nlohmann::json& config, const context_shared_ptr& ctx, const std::vector<tool_info>& tools)
{
	auto go{ make_go(config, ctx, tools) };

	auto client{ ai::openai::create_client(config["api_key"].get<std::string>(), config["base_url"].get<std::string>()) };

	auto result{ client.generate_text(go) };
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

		resp->message = assistant_message;
	}

	return resp;
}

std::string_view chat_completion_api_factory::name() const
{
	return "chat_completion_api";
}

api_unique_ptr chat_completion_api_factory::create() const
{
	return std::make_unique<chat_completion_api>();
}

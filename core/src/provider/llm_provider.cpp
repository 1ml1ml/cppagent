module;

#include <string>
#include <memory>
#include <stdexcept>

#include "nlohmann/json.hpp"

module llm_provider;

import context;
import message;
import mcp_manager;

bool model_normal_response::is_truncated() const
{
	return finish_reason == "length";
}

void model_normal_response::apply_to_context(const context_shared_ptr& ctx, const mcp_manager_shared_ptr&) const
{
	ctx->append(message);
}

void model_tool_call_response::apply_to_context(const context_shared_ptr& ctx, const mcp_manager_shared_ptr& mcp) const
{
	if (!mcp)
	{
		throw std::runtime_error("model response indicates a tool call, but no mcp manager was provided");
	}

	for (const auto& tool_call : tool_calls)
	{
		auto result{ mcp->call_tool(tool_call.function_name, tool_call.arguments) };
		result["tool_call_id"] = tool_call.id;
		ctx->append(std::make_shared<class message>(message::role::tool, result));
	}
}

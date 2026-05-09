module;

#include <memory>
#include <string>

module llm_provider;

import context;
import message;

bool model_text_response::is_truncated() const
{
	return finish_reason == "length";
}

void model_text_response::apply_to_context(const context_shared_ptr& ctx) const
{
	ctx->append(message);
}

void model_tool_call_response::apply_to_context(const context_shared_ptr& ctx) const
{
	// TODO: 工具调用结果回写 context
}

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
};

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
	return {};
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

module;

#include <memory>

#include "nlohmann/json.hpp"

module agent;

import llm_api;
import mcp_client;
import mcp_manager;
import api_registry;
import skill_manager;

class agent::impl
{
public:
	nlohmann::json model_config{};
	mcp_manager_shared_ptr mcp_manager{};
	skill_manager_shared_ptr skill_manager{};
};

agent::agent() :
	impl{ std::make_unique<class impl>() }
{
}

agent::~agent() = default;

nlohmann::json agent::get_model_config() const
{
	return impl->model_config;
}

void agent::set_model_config(const nlohmann::json& config)
{
	impl->model_config = config;
}

mcp_manager_shared_ptr agent::get_mcp_manager() const
{
	return impl->mcp_manager;
}

void agent::set_mcp_manager(const mcp_manager_shared_ptr& manager)
{
	impl->mcp_manager = manager;
}

skill_manager_shared_ptr agent::get_skill_manager() const
{
	return impl->skill_manager;
}

void agent::set_skill_manager(const skill_manager_shared_ptr& manager)
{
	impl->skill_manager = manager;
}

model_response_shared_ptr agent::generate_text(const context_shared_ptr& ctx)
{
	auto api{ api_registry::instance().create(impl->model_config["api"].get<std::string>()) };
	auto tools{ impl->mcp_manager ? impl->mcp_manager->get_tools() : std::vector<tool_shared_ptr>{} };

	while (true)
	{
		auto resp{ api->generate_text(impl->model_config, ctx, tools) };

		ctx->set_instructions(impl->skill_manager ? impl->skill_manager->catalog_text() : "");
		ctx->append(resp->message);

		if (resp->finish_reason == "tool_calls")
		{
			auto assistant_message{ std::static_pointer_cast<class assistant_message>(resp->message) };

			std::vector<tool_call_result> tool_call_results{};
			for (const auto& tool_call : assistant_message->get_tool_calls_ref())
			{
				for (const auto& tool : tools)
				{
					if (tool_call.tool_name == tool->name)
					{
						try
						{
							auto tool_result{ tool->call(tool_call.arguments) };
							tool_call_results.emplace_back(tool_call.id, tool_result.is_error, tool_result.content);
						}
						catch (const std::exception& ex)
						{
							tool_call_results.emplace_back(tool_call.id, true, ex.what());
						}
					}
					break;
				}
			}

			auto user_message{ std::make_shared<class user_message>() };
			user_message->set_tool_call_results(tool_call_results);
			ctx->append(user_message);

			continue;
		}

		return resp;
	}
}

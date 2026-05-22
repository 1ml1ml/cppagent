module;

#include <memory>

#include "nlohmann/json.hpp"

export module agent;

import context;
import llm_api;
import message;
import mcp_manager;
import skill_manager;

export class agent
{
public:
	agent();
	~agent();

public:
	nlohmann::json get_model_config() const;
	void set_model_config(const nlohmann::json& config);

	mcp_manager_shared_ptr get_mcp_manager() const;
	void set_mcp_manager(const mcp_manager_shared_ptr& manager);

	skill_manager_shared_ptr get_skill_manager() const;
	void set_skill_manager(const skill_manager_shared_ptr& manager);

	model_response_shared_ptr generate_text(const context_shared_ptr& ctx);

private:
	class impl;
	std::unique_ptr<impl> impl{};
};

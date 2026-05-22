module;

#include <memory>
#include <string>

#include "nlohmann/json.hpp"

export module chat_completion_api;

import context;
import llm_api;
import mcp_client;

export class chat_completion_api : public llm_api
{
public:
  model_response_shared_ptr generate_text(nlohmann::json& config, const context_shared_ptr& ctx, const std::vector<tool_shared_ptr>& tools = {}) override;
};

export class chat_completion_api_factory : public llm_api_factory
{
public:
  std::string_view name() const override;
  api_unique_ptr create() const override;
};

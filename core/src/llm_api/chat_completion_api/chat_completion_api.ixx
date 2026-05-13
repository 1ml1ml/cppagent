module;

#include <memory>
#include <string>

#include "nlohmann/json.hpp"

export module chat_completion_api;

import llm_api;

import context;
import mcp_client;

export class chat_completion_api : public llm_api
{
public:
  chat_completion_api();
  ~chat_completion_api();

public:
  nlohmann::json get_config() const override;
  void set_config(const nlohmann::json& config) override;

  model_response_shared_ptr generate_text(const context_shared_ptr& ctx, const std::vector<tool_info>& tools = {}) override;

private:
  class impl;
  std::unique_ptr<impl> impl{};
};

export class chat_completion_api_factory : public llm_api_factory
{
public:
  std::string_view name() const override;
  api_unique_ptr create() const override;
};

module;

#include <memory>
#include <string>

#include "nlohmann/json.hpp"

export module openai_responses_provider;

import llm_provider;

import context;
import mcp_client;

export class openai_responses_provider : public llm_provider
{
public:
  openai_responses_provider();
  ~openai_responses_provider();

public:
  nlohmann::json get_config() const override;
  void set_config(const nlohmann::json& config) override;

  model_response_shared_ptr generate(const context_shared_ptr& ctx, const std::vector<tool_info>& tools = {}, const stream_callback& callback = {}) override;
  std::future<model_response_shared_ptr> generate_async(const context_shared_ptr& ctx, const std::vector<tool_info>& tools = {}, const stream_callback& callback = {}) override;

private:
  class impl;
  std::unique_ptr<impl> impl{};
};

export class openai_responses_provider_factory : public llm_provider_factory
{
public:
  std::string_view name() const override;
  provider_unique_ptr create() const override;
};

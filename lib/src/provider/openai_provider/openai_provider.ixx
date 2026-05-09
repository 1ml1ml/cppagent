module;

#include <memory>
#include <string>

#include "nlohmann/json.hpp"

export module openai_provider;

import llm_provider;

import context;

export class openai_provider final : public llm_provider
{
public:
  openai_provider();
  ~openai_provider();

public:
  nlohmann::json get_config() const override;
  void set_config(const nlohmann::json& config) override;

  model_response_shared_ptr generate( const context_shared_ptr& ctx, const stream_callback& callback = {}) override;
  std::future<model_response_shared_ptr> generate_async( const context_shared_ptr& ctx, const stream_callback& callback = {}) override;

private:
  class impl;
  std::unique_ptr<impl> impl{};
};

export class openai_factory final : public llm_provider_factory
{
public:
  std::string_view name() const override;
  provider_unique_ptr create() const override;
};

module;

#include <memory>
#include <string>

#include "nlohmann/json.hpp"

export module llm_api;

import core;

export class llm_api
{
public:
  virtual ~llm_api() = default;

  virtual model_response_shared_ptr generate_text(nlohmann::json& config, const context_shared_ptr& ctx, const std::vector<tool_shared_ptr>& tools = {}) = 0;
};

export class llm_api_factory : public std::enable_shared_from_this<llm_api_factory>
{
public:
  virtual ~llm_api_factory() = default;

  virtual std::string_view name() const = 0;
  virtual api_unique_ptr create() const = 0;
};

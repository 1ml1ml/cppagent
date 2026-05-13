module;

#include <functional>
#include <future>
#include <memory>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

export module llm_api;

import context;
import message;
import mcp_client;
import mcp_manager;

export struct usage_info
{
  int total_tokens{};
  int prompt_tokens{};
  int completion_tokens{};
};

export struct tool_call
{
  std::string id{};
  std::string type{};
  nlohmann::json arguments{};
  std::string function_name{};
};

export class model_response
{
public:
  std::string id{};
  std::string model{};
  std::string finish_reason{};

  usage_info usage{};
  message_shared_ptr message{};
};
export using model_response_shared_ptr = std::shared_ptr<model_response>;

class llm_api;
export using api_unique_ptr = std::unique_ptr<llm_api>;

export class llm_api
{
public:
  virtual ~llm_api() = default;

  virtual nlohmann::json get_config() const = 0;
  virtual void set_config(const nlohmann::json& config) = 0;

  virtual model_response_shared_ptr generate_text( const context_shared_ptr& ctx, const std::vector<tool_info>& tools = {}) = 0;
};

class llm_api_factory;
export using api_factory_shared_ptr = std::shared_ptr<llm_api_factory>;

export class llm_api_factory : public std::enable_shared_from_this<llm_api_factory>
{
public:
  virtual ~llm_api_factory() = default;

  virtual std::string_view name() const = 0;
  virtual api_unique_ptr create() const = 0;
};

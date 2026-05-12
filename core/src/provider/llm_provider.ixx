module;

#include <functional>
#include <future>
#include <memory>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

export module llm_provider;

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

class llm_provider;
export using provider_unique_ptr = std::unique_ptr<llm_provider>;

export class llm_provider
{
public:
	using stream_callback = std::function<bool(std::string)>;

public:
  virtual ~llm_provider() = default;

  virtual nlohmann::json get_config() const = 0;
  virtual void set_config(const nlohmann::json& config) = 0;

  virtual model_response_shared_ptr generate( const context_shared_ptr& ctx, const std::vector<tool_info>& tools = {}, const stream_callback& callback = {}) = 0;
  virtual std::future<model_response_shared_ptr> generate_async( const context_shared_ptr& ctx, const std::vector<tool_info>& tools = {}, const stream_callback& callback = {}) = 0;
};

class llm_provider_factory;
export using provider_factory_shared_ptr = std::shared_ptr<llm_provider_factory>;

export class llm_provider_factory : public std::enable_shared_from_this<llm_provider_factory>
{
public:
  virtual ~llm_provider_factory() = default;

  virtual std::string_view name() const = 0;
  virtual provider_unique_ptr create() const = 0;
};

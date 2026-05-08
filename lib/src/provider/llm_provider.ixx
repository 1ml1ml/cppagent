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

export struct usage_info
{
  int total_tokens{};
  int prompt_tokens{};
  int completion_tokens{};
};

export struct tool_call
{
  std::string id{};
  std::string type{};           // "function"
  nlohmann::json arguments{};
  std::string function_name{};
};

export class generation_result_base
{
public:
  virtual ~generation_result_base() = default;

  std::string id{};
  std::string model{};
};
export using generation_result_shared_ptr = std::shared_ptr<generation_result_base>;

export class text_result : public generation_result_base
{
public:
  usage_info usage{};
  std::string finish_reason{};  // "stop" / "length"
  message_shared_ptr message{};

  bool is_truncated() const
  {
    return finish_reason == "length";
  }
};

export class tool_result : public generation_result_base
{
public:
  usage_info usage{};
  message_shared_ptr message{};  // 可选：思考过程（如 CoT）
  std::vector<tool_call> tool_calls{};
};

export using stream_callback = std::function<bool(std::string data)>;

class llm_provider;
export using provider_unique_ptr = std::unique_ptr<llm_provider>;

export class llm_provider
{
public:
  virtual ~llm_provider() = default;

  virtual nlohmann::json get_config() const = 0;
  virtual void set_config(const nlohmann::json& config) = 0;

  virtual generation_result_shared_ptr generate( const context_shared_ptr& ctx, const stream_callback& callback = {}) = 0;
  virtual std::future<generation_result_shared_ptr> generate_async( const context_shared_ptr& ctx, const stream_callback& callback = {}) = 0;
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

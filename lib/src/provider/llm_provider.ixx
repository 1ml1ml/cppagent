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

// ── 计量信息 ──
export struct usage_info
{
  int prompt_tokens{0};
  int completion_tokens{0};
  int total_tokens{0};
};

// ── 工具调用 ──
export struct tool_call
{
  std::string id{};
  std::string type{};           // "function"
  std::string function_name{};
  nlohmann::json arguments{};
};

// ── 结果基类 ──
export class result_base
{
public:
  virtual ~result_base() = default;

  std::string id{};
  std::string model{};
};

// ── 文本回复结果 ──
export class text_result : public result_base
{
public:
  message_shared_ptr message{};
  usage_info usage{};
  std::string finish_reason{};  // "stop" / "length"

  bool is_truncated() const
  {
    return finish_reason == "length";
  }
};

// ── 工具调用结果 ──
export class tool_result : public result_base
{
public:
  std::vector<tool_call> tool_calls{};
  message_shared_ptr message{};  // 可选：思考过程（如 CoT）
  usage_info usage{};
};

// ── provider 接口 ──
export using stream_callback = std::function<bool(std::string data)>;

class llm_provider;
export using provider_unique_ptr = std::unique_ptr<llm_provider>;

export class llm_provider
{
public:
  virtual ~llm_provider() = default;

  virtual nlohmann::json get_config() const = 0;
  virtual void set_config(const nlohmann::json& config) = 0;

  virtual std::shared_ptr<result_base> generate(
      const context_shared_ptr& ctx,
      const stream_callback& callback = {}) = 0;

  virtual std::future<std::shared_ptr<result_base>> generate_async(
      const context_shared_ptr& ctx,
      const stream_callback& callback = {}) = 0;
};

// ── 工厂接口 ──
class llm_provider_factory;
export using provider_factory_shared_ptr = std::shared_ptr<llm_provider_factory>;

export class llm_provider_factory : public std::enable_shared_from_this<llm_provider_factory>
{
public:
  virtual ~llm_provider_factory() = default;

  virtual std::string_view name() const = 0;
  virtual provider_unique_ptr create() const = 0;
};

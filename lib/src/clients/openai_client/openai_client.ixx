module;

#include <memory>
#include <future>
#include <string>
#include <functional>

#include "nlohmann/json.hpp"

export module openai_client;

import i_client;

import context;

export class openai_client final : public i_client
{
public:
  openai_client();
  ~openai_client();

public:
  context_shared_ptr generate(const nlohmann::json& config, const context_shared_ptr& ctx, const std::function<bool(std::string data)>& stream_callback = {}) override;
  std::future<context_shared_ptr> generate_async(const nlohmann::json& config, const context_shared_ptr& ctx, const std::function<bool(std::string data)>& stream_callback = {}) override;

private:
  class impl;
  std::unique_ptr<impl> impl{};
};

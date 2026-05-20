module;

#include <chrono>
#include <memory>
#include <vector>
#include <string_view>

#include "nlohmann/json.hpp"

export module mcp_manager;

import core;

export class mcp_manager : public std::enable_shared_from_this<mcp_manager>
{
public:
  mcp_manager();
  ~mcp_manager();

public:
  void load(const nlohmann::json& config);

  std::vector<tool_shared_ptr> get_tools(const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 5000 }) const;

private:
  class impl;
  std::unique_ptr<impl> impl{};
};

module;

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <chrono>

#include "nlohmann/json.hpp"

export module mcp_manager;

import mcp_client;

class mcp_manager;
export using mcp_manager_shared_ptr = std::shared_ptr<mcp_manager>;

export class mcp_manager : public std::enable_shared_from_this<mcp_manager>
{
public:
  mcp_manager();
  ~mcp_manager();

public:
  void load(const nlohmann::json& config);

  std::vector<tool_info> get_tools(const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 5000 }) const;

private:
  class impl;
  std::unique_ptr<impl> impl{};
};

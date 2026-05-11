module;

#include <map>
#include <memory>
#include <string>
#include <vector>

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

  nlohmann::json get_tools_schema() const;
  nlohmann::json call_tool(const std::string& prefixed_name, const nlohmann::json& arguments) const;

private:
  class impl;
  std::unique_ptr<impl> impl{};
};

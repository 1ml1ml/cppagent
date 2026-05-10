module;

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

export module mcp_client;

import transport;

export struct tool_info
{
  std::string name{};
  std::string description{};
  nlohmann::json input_schema{};
};

export struct tool_result
{
  bool is_error{};
  nlohmann::json content{};
};

export class mcp_client final
{
public:
  mcp_client(transport_unique_ptr&& transport);
  ~mcp_client();

public:
	std::string get_name() const;
	void set_name(const std::string_view& name);

  std::string get_version() const;
	void set_version(const std::string_view& version);

  void initialize(const nlohmann::json& capabilities, const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 30 });

  std::vector<tool_info> list_tools(const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 30 });
  tool_result call_tool(const std::string& name, const nlohmann::json& arguments, const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 30 });

private:
  class impl;
  std::unique_ptr<impl> impl{};
};

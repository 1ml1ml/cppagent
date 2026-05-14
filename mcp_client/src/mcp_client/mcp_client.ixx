module;

#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <string_view>

#include "nlohmann/json.hpp"

export module mcp_client;

import transport;

class mcp_client;
export using mcp_client_shared_ptr = std::shared_ptr<mcp_client>;

export struct tool_result
{
  bool is_error{};
  nlohmann::json content{};
};

export struct tool_info
{
public:
  std::string name{};
  std::string description{};
  nlohmann::json input_schema{};
};

export struct resource_info
{
  std::string uri{};
  std::string name{};
  std::string mime_type{};
  std::string description{};
};

export struct resource_content
{
  std::string uri{};
  std::string mime_type{};

  std::string text{};
  std::string blob{};
};

export using request_handler = std::function<nlohmann::json(const nlohmann::json& params)>;
export using notification_handler = std::function<void(const nlohmann::json& params)>;

export class mcp_client : public std::enable_shared_from_this<mcp_client>
{
public:
  mcp_client(transport_unique_ptr&& transport);
  ~mcp_client();

public:
	std::string_view get_name() const;
	void set_name(const std::string_view& name);

  std::string_view get_version() const;
	void set_version(const std::string_view& version);

  void initialize(const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 5000 });

  std::vector<tool_info> list_tools(const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 5000 });
  tool_result call_tool(const std::string_view& name, const nlohmann::json& arguments, const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 5000 });

  std::vector<resource_info> list_resources(const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 5000 });
  std::vector<resource_content> read_resource(const std::string_view& uri, const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 5000 });

private:
  class impl;
  std::unique_ptr<impl> impl{};
};

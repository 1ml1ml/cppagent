module;

#include <chrono>
#include <functional>
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

class mcp_client;
export using mcp_client_shared_ptr = std::shared_ptr<mcp_client>;

export class mcp_client : public std::enable_shared_from_this<mcp_client>
{
public:
  mcp_client(transport_unique_ptr&& transport);
  ~mcp_client();

public:
	std::string get_name() const;
	void set_name(const std::string_view& name);

  std::string get_version() const;
	void set_version(const std::string_view& version);

  void initialize(const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 5000 });

  std::vector<tool_info> list_tools(const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 5000 });
  tool_result call_tool(const std::string& name, const nlohmann::json& arguments, const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 5000 });

  std::vector<resource_info> list_resources(const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 5000 });
  std::vector<resource_content> read_resource(const std::string& uri, const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 5000 });

private:
  class impl;
  std::unique_ptr<impl> impl{};
};

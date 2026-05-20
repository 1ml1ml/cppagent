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

class tool;
export using tool_shared_ptr = std::shared_ptr<tool>;


export class tool : public std::enable_shared_from_this<tool>
{
public:
	struct result
	{
		bool is_error{};
		nlohmann::json content{};
	};

public:
  virtual ~tool() = default;

public:
  virtual result call(const nlohmann::json& arguments, const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 5000 }) const = 0;

public:
  std::string name{};
  std::string description{};
  nlohmann::json input_schema{};
};

export class mcp_client_tool_info : public tool
{
public:
  mcp_client_tool_info(const mcp_client_shared_ptr& client);

public:
  tool::result call(const nlohmann::json& arguments, const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 5000 }) const override;

private:
  mcp_client_shared_ptr client{};
};

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

  std::vector<tool_shared_ptr> list_tools(const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 5000 });
  tool::result call_tool(const std::string_view& name, const nlohmann::json& arguments, const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 5000 });

private:
  class impl;
  std::unique_ptr<impl> impl{};
};

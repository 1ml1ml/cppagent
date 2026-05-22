module;

#include <map>
#include <string>
#include <format>
#include <chrono>
#include <vector>
#include <stdexcept>
#include <string_view>

#include "nlohmann/json.hpp"

module mcp_manager;

import mcp_client;
import stdio_transport;

class mcp_manager::impl
{
public:
	std::map<std::string, mcp_client_shared_ptr> clients{};
};

mcp_manager::mcp_manager()
  : impl{ std::make_unique<class impl>() }
{
}

mcp_manager::~mcp_manager() = default;

void mcp_manager::load(const nlohmann::json& config)
{
  for (const auto& [server_name, server_config] : config.items())
  {
		auto client{ std::make_shared<mcp_client>(std::make_unique<stdio_transport>(server_config["command"].get<std::string>(), server_config.value("args", std::vector<std::string>{}), server_config.value("env", std::map<std::string, std::string>{}))) };
		client->set_name("cppagent.exe");
		client->set_version("1.0");
		client->initialize();
		impl->clients[server_name] = client;
  }
}

std::vector<tool_shared_ptr> mcp_manager::list_tools(const std::chrono::milliseconds& timeout) const
{
	std::vector<tool_shared_ptr> tools{};
	for (const auto& [server_name, client] : impl->clients)
	{
		tools.insert_range(tools.end(), client->list_tools());
	}
	return tools;
}

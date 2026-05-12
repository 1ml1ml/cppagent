module;

#include <map>
#include <chrono>
#include <vector>
#include <stdexcept>

#include "nlohmann/json.hpp"

module mcp_manager;

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
  for (const auto& [server_name, server_config] : config["mcpServers"].items())
  {
		auto client{ std::make_shared<mcp_client>(std::make_unique<stdio_transport>(server_config["command"].get<std::string>(), server_config.value("args", std::vector<std::string>{}), server_config.value("env", std::map<std::string, std::string>{}))) };
		client->set_name("cppagent.exe");
		client->set_version("1.0");
		client->initialize();
		impl->clients[server_name] = client;
  }
}

std::vector<tool_info> mcp_manager::get_tools(const std::chrono::milliseconds& timeout) const
{
	std::vector<tool_info> tools{};
	for (const auto& [server_name, client] : impl->clients)
	{
		for (auto tool : client->list_tools(timeout))
		{
			tools.push_back(tool);
		}
	}
	return tools;
}

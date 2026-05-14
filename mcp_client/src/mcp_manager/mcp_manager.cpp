module;

#include <map>
#include <format>
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
			tool.name = std::format("{}_{}", server_name, tool.name);
			tools.push_back(std::move(tool));
		}
	}
	return tools;
}

tool_result mcp_manager::call_tool(const std::string_view& name, const nlohmann::json& arguments, const std::chrono::milliseconds& timeout) const
{
	auto pos{ name.find('_') };
	if (pos == std::string_view::npos)
	{
		throw std::runtime_error{ std::format("call_tool: invalid tool name '{}', expected format 'server_name/tool_name'", name) };
	}

	auto server_name{ name.substr(0, pos) };
	auto tool_name{ name.substr(pos + 1) };

	auto it{ impl->clients.find(std::string(server_name)) };
	if (it == impl->clients.end())
	{
		throw std::runtime_error{ std::format("call_tool: server '{}' not found", server_name) };
	}

	return it->second->call_tool(tool_name, arguments, timeout);
}

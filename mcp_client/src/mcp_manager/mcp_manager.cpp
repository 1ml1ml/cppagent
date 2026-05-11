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

		if (server_config.contains("roots"))
		{
			nlohmann::json roots{ nlohmann::json::array() };
			for (const auto& path : server_config["roots"])
			{
				nlohmann::json item{};
				item["uri"] = "file:///" + path.get<std::string>();
				item["name"] = path.get<std::string>();
				roots.push_back(std::move(item));
			}
			client->set_roots(roots);
		}

		client->initialize();
		impl->clients[server_name] = client;
  }
}

nlohmann::json mcp_manager::get_tools_schema() const
{
	nlohmann::json tools{ nlohmann::json::array() };

	for (const auto& [server_name, client] : impl->clients)
	{
		for (const auto& tool : client->list_tools())
		{
			nlohmann::json item{ nlohmann::json::object() };
			item["type"] = "function";
			item["function"]["name"] = server_name + "/" + tool.name;
			item["function"]["description"] = tool.description;
			item["function"]["parameters"] = tool.input_schema;
			tools.push_back(std::move(item));
		}
	}

	return tools;
}

nlohmann::json mcp_manager::call_tool(const std::string& prefixed_name, const nlohmann::json& arguments) const
{
	auto pos{ prefixed_name.find('/') };
	if (pos == std::string::npos)
	{
		throw std::runtime_error{ "Invalid tool name format, expected 'server/tool': " + prefixed_name };
	}

	auto server_name{ prefixed_name.substr(0, pos) };
	auto tool_name{ prefixed_name.substr(pos + 1) };

	auto it{ impl->clients.find(server_name) };
	if (it == impl->clients.end())
	{
		throw std::runtime_error{ "Unknown MCP server: " + server_name };
	}

	auto result{ it->second->call_tool(tool_name, arguments) };

	nlohmann::json response{ nlohmann::json::object() };
	response["is_error"] = result.is_error;
	response["content"] = result.content;

	return response;
}

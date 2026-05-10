module;

#include <chrono>
#include <future>
#include <variant>
#include <type_traits>
#include <vector>

#include "nlohmann/json.hpp"

module mcp_client;

import jsonrpc;

class mcp_client::impl
{
public:
	jsonrpc_response send(const jsonrpc_request& req, const std::chrono::milliseconds& timeout);

public:
	std::string name{};
	std::string version{};

	bool initialized{ false };
	transport_unique_ptr transport{};
	std::atomic<std::int64_t> next_request_id{ 1 };
	std::promise<jsonrpc_response> response_promise{};
};

jsonrpc_response mcp_client::impl::send(const jsonrpc_request& req, const std::chrono::milliseconds& timeout)
{
	response_promise = std::promise<jsonrpc_response>{};

	transport->send(req);

	auto response_future{ response_promise.get_future() };
	if (response_future.wait_for(timeout) == std::future_status::timeout)
	{
		throw std::runtime_error{ "Request timed out" };
	}

	return response_future.get();
}

mcp_client::mcp_client(transport_unique_ptr&& transport)
	: impl{ std::make_unique<class impl>() }
{
	impl->transport = std::move(transport);

	impl->transport->set_receive_callback([this](const jsonrpc_message& msg)
		{
			std::visit([this](const auto& jsonrpc)
				{
					if constexpr (std::derived_from<std::decay_t<decltype(jsonrpc)>, jsonrpc_response>)
					{
						impl->response_promise.set_value(jsonrpc);
					}
				}, msg);
		});
}

mcp_client::~mcp_client() = default;

std::string mcp_client::get_name() const
{
	return impl->name;
}

void mcp_client::set_name(const std::string_view& name)
{
	impl->name = name;
}

std::string mcp_client::get_version() const
{
	return impl->version;
}

void mcp_client::set_version(const std::string_view& version)
{
	impl->version = version;
}

void mcp_client::initialize(const nlohmann::json& capabilities, const std::chrono::milliseconds& timeout)
{
	if (impl->initialized)
	{
		throw std::runtime_error{ "Client already initialized" };
	}

	if (!impl->transport->is_connected())
	{
		impl->transport->open();
	}

	jsonrpc_request req{};
	req.id = impl->next_request_id.fetch_add(1, std::memory_order_relaxed);
	req.method = "initialize";

	req.params = nlohmann::json::object();
	req.params["protocolVersion"] = "2024-11-05";
	req.params["capabilities"] = capabilities;

	req.params["clientInfo"] = nlohmann::json::object();
	req.params["clientInfo"]["name"] = impl->name;
	req.params["clientInfo"]["version"] = impl->version;

	auto response{ impl->send(req, timeout) };
	if (!response.payload.has_value())
	{
		throw std::runtime_error{ "Initialization failed: " + response.payload.error().message };
	}

	jsonrpc_notification notify{};
	notify.method = "notifications/initialized";
	impl->transport->send(notify);

	impl->initialized = true;
}

std::vector<tool_info> mcp_client::list_tools(const std::chrono::milliseconds& timeout)
{
	if (!impl->initialized)
	{
		throw std::runtime_error{ "Client not initialized" };
	}

	jsonrpc_request req{};
	req.id = impl->next_request_id.fetch_add(1, std::memory_order_relaxed);
	req.method = "tools/list";

	auto response{ impl->send(req, timeout) };
	if (!response.payload.has_value())
	{
		throw std::runtime_error{ "list_tools failed: " + response.payload.error().message };
	}

	std::vector<tool_info> tools{};
	for (const auto& tool : response.payload.value()["tools"])
	{
		tool_info info{};
		info.name = tool["name"].get<std::string>();
		info.description = tool["description"].get<std::string>();
		info.input_schema = tool["inputSchema"];
		tools.push_back(std::move(info));
	}

	return tools;
}

tool_result mcp_client::call_tool(const std::string& name, const nlohmann::json& arguments, const std::chrono::milliseconds& timeout)
{
	if (!impl->initialized)
	{
		throw std::runtime_error{ "Client not initialized" };
	}

	jsonrpc_request req{};
	req.id = impl->next_request_id.fetch_add(1, std::memory_order_relaxed);
	req.method = "tools/call";

	req.params = nlohmann::json::object();
	req.params["name"] = name;
	req.params["arguments"] = arguments;

	auto response{ impl->send(req, timeout) };
	if (!response.payload.has_value())
	{
		throw std::runtime_error{ "call_tool failed: " + response.payload.error().message };
	}

	auto data{ response.payload.value() };

	tool_result tr{};
	tr.is_error = data.value("isError", false);
	tr.content = data["content"];
	return tr;
}

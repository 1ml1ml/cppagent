module;

#include <chrono>
#include <expected>
#include <future>
#include <map>
#include <shared_mutex>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include "nlohmann/json.hpp"

module mcp_client;

import jsonrpc;

tool_result tool_info::exec(const nlohmann::json& arguments, const std::chrono::milliseconds& timeout) const
{
	return client->call_tool(name, arguments, timeout);
}

class mcp_client::impl
{
public:
	jsonrpc_response send(const jsonrpc_request& req, const std::chrono::milliseconds& timeout);

	void handle_response(const jsonrpc_response& resp);

public:
	std::string name{};
	std::string version{};

	transport_unique_ptr transport{};
	std::atomic_bool initialized{ false };

	std::atomic<std::int64_t> next_request_id{ 1 };

	std::shared_mutex pending_responses_mutex{};
	std::map<std::int64_t, std::promise<jsonrpc_response>> pending_responses{};
};

jsonrpc_response mcp_client::impl::send(const jsonrpc_request& req, const std::chrono::milliseconds& timeout)
{
	pending_responses_mutex.lock();
	auto response_future{ (pending_responses[req.id] = std::promise<jsonrpc_response>{}).get_future() };
	pending_responses_mutex.unlock();

	transport->send(req);
	auto status{ response_future.wait_for(timeout) };

	pending_responses_mutex.lock();
	pending_responses.erase(req.id);
	pending_responses_mutex.unlock();

	if (status == std::future_status::timeout)
	{
		throw std::runtime_error{ "Request timed out" };
	}

	return response_future.get();
}

void mcp_client::impl::handle_response(const jsonrpc_response& resp)
{
	std::shared_lock lock{ pending_responses_mutex };
	if (auto it{ pending_responses.find(resp.id) }; it != pending_responses.end())
	{
		it->second.set_value(resp);
	}
}

mcp_client::mcp_client(transport_unique_ptr&& transport)
	: impl{ std::make_unique<class impl>() }
{
	impl->transport = std::move(transport);

	impl->transport->set_receive_callback([this](const jsonrpc_message& msg)
		{
			std::visit([this](const auto& jsonrpc)
				{
					using T = std::decay_t<decltype(jsonrpc)>;
					if constexpr (std::derived_from<T, jsonrpc_response>)
					{
						impl->handle_response(jsonrpc);
					}
				}, msg);
		});
}

mcp_client::~mcp_client() = default;

std::string_view mcp_client::get_name() const
{
	return impl->name;
}

void mcp_client::set_name(const std::string_view& name)
{
	impl->name = name;
}

std::string_view mcp_client::get_version() const
{
	return impl->version;
}

void mcp_client::set_version(const std::string_view& version)
{
	impl->version = version;
}

void mcp_client::initialize(const std::chrono::milliseconds& timeout)
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
	req.params["protocolVersion"] = "2024-11-05";
	req.params["clientInfo"]["name"] = impl->name;
	req.params["clientInfo"]["version"] = impl->version;
	req.params["capabilities"]["roots"]["listChanged"] = false;

	auto resp{ impl->send(req, timeout) };
	if (!resp.payload.has_value())
	{
		throw std::runtime_error{ "Initialization failed: " + resp.payload.error().message };
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

	auto resp{ impl->send(req, timeout) };
	if (!resp.payload.has_value())
	{
		throw std::runtime_error{ "list_tools failed: " + resp.payload.error().message };
	}

	std::vector<tool_info> tools{};
	for (const auto& tool_json : resp.payload.value()["tools"])
	{
		tool_info tool{};
		tool.client = shared_from_this();
		tool.name = tool_json["name"].get<std::string>();
		tool.description = tool_json["description"].get<std::string>();
		tool.input_schema = tool_json["inputSchema"];
		tools.push_back(std::move(tool));
	}

	return tools;
}

tool_result mcp_client::call_tool(const std::string_view& name, const nlohmann::json& arguments, const std::chrono::milliseconds& timeout)
{
	if (!impl->initialized)
	{
		throw std::runtime_error{ "Client not initialized" };
	}

	jsonrpc_request req{};
	req.id = impl->next_request_id.fetch_add(1, std::memory_order_relaxed);
	req.method = "tools/call";
	req.params["name"] = name;
	req.params["arguments"] = arguments;

	auto resp{ impl->send(req, timeout) };
	if (!resp.payload.has_value())
	{
		throw std::runtime_error{ "call_tool failed: " + resp.payload.error().message };
	}

	auto resp_json{ resp.payload.value() };

	tool_result tr{};
	tr.is_error = resp_json.value("isError", false);
	tr.content = resp_json["content"];
	return tr;
}

std::vector<resource_info> mcp_client::list_resources(const std::chrono::milliseconds& timeout)
{
	if (!impl->initialized)
	{
		throw std::runtime_error{ "Client not initialized" };
	}

	jsonrpc_request req{};
	req.id = impl->next_request_id.fetch_add(1, std::memory_order_relaxed);
	req.method = "resources/list";

	auto resp{ impl->send(req, timeout) };
	if (!resp.payload.has_value())
	{
		throw std::runtime_error{ "list_resources failed: " + resp.payload.error().message };
	}

	std::vector<resource_info> resources{};
	for (const auto& resource_json : resp.payload.value()["resources"])
	{
		resource_info resource{};
		resource.uri = resource_json["uri"].get<std::string>();
		resource.name = resource_json["name"].get<std::string>();

		if (resource_json.contains("mimeType"))
		{
			resource.mime_type = resource_json["mimeType"].get<std::string>();
		}

		if (resource_json.contains("description"))
		{
			resource.description = resource_json["description"].get<std::string>();
		}

		resources.push_back(std::move(resource));
	}

	return resources;
}

std::vector<resource_content> mcp_client::read_resource(const std::string_view& uri, const std::chrono::milliseconds& timeout)
{
	if (!impl->initialized)
	{
		throw std::runtime_error{ "Client not initialized" };
	}

	jsonrpc_request req{};
	req.id = impl->next_request_id.fetch_add(1, std::memory_order_relaxed);
	req.method = "resources/read";
	req.params["uri"] = uri;

	auto resp{ impl->send(req, timeout) };
	if (!resp.payload.has_value())
	{
		throw std::runtime_error{ "read_resource failed: " + resp.payload.error().message };
	}

	std::vector<resource_content> contents{};
	for (const auto& content_json : resp.payload.value()["contents"])
	{
		resource_content content{};
		content.uri = content_json["uri"].get<std::string>();

		if (content_json.contains("mimeType"))
		{
			content.mime_type = content_json["mimeType"].get<std::string>();
		}

		if (content_json.contains("text"))
		{
			content.text = content_json["text"].get<std::string>();
		}

		if (content_json.contains("blob"))
		{
			content.blob = content_json["blob"].get<std::string>();
		}

		contents.push_back(std::move(content));
	}

	return contents;
}

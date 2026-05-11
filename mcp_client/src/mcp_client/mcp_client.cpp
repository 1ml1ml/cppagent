module;

#include <vector>
#include <chrono>
#include <future>
#include <variant>
#include <expected>
#include <type_traits>
#include <shared_mutex>

#include "nlohmann/json.hpp"

module mcp_client;

import jsonrpc;

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
					else if constexpr (std::derived_from<T, jsonrpc_request>)
					{
					}
					else if constexpr (std::derived_from<T, jsonrpc_notification>)
					{
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
	req.params["name"] = name;
	req.params["arguments"] = arguments;

	auto response{ impl->send(req, timeout) };
	if (!response.payload.has_value())
	{
		throw std::runtime_error{ "call_tool failed: " + response.payload.error().message };
	}

	auto json{ response.payload.value() };

	tool_result tr{};
	tr.is_error = json.value("isError", false);
	tr.content = json["content"];
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

	auto response{ impl->send(req, timeout) };
	if (!response.payload.has_value())
	{
		throw std::runtime_error{ "list_resources failed: " + response.payload.error().message };
	}

	std::vector<resource_info> resources{};
	for (const auto& item : response.payload.value()["resources"])
	{
		resource_info info{};
		info.uri = item["uri"].get<std::string>();
		info.name = item["name"].get<std::string>();

		if (item.contains("mimeType"))
		{
			info.mime_type = item["mimeType"].get<std::string>();
		}

		if (item.contains("description"))
		{
			info.description = item["description"].get<std::string>();
		}

		resources.push_back(std::move(info));
	}

	return resources;
}

std::vector<resource_content> mcp_client::read_resource(const std::string& uri, const std::chrono::milliseconds& timeout)
{
	if (!impl->initialized)
	{
		throw std::runtime_error{ "Client not initialized" };
	}

	jsonrpc_request req{};
	req.id = impl->next_request_id.fetch_add(1, std::memory_order_relaxed);
	req.method = "resources/read";
	req.params["uri"] = uri;

	auto response{ impl->send(req, timeout) };
	if (!response.payload.has_value())
	{
		throw std::runtime_error{ "read_resource failed: " + response.payload.error().message };
	}

	std::vector<resource_content> contents{};
	for (const auto& content_json : response.payload.value()["contents"])
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

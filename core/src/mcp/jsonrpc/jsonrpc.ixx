module;

#include <string>
#include <cstdint>
#include <variant>
#include <expected>

#include "nlohmann/json.hpp"

export module jsonrpc;

export enum class error_code : std::int32_t
{
	PARSE_ERROR = -32700,
	INVALID_REQUEST = -32600,
	METHOD_NOT_FOUND = -32601,
	INVALID_PARAMS = -32602,
	INTERNAL_ERROR = -32603,
};

export struct jsonrpc_error
{
public:
	static jsonrpc_error from_json(const nlohmann::json& j);

public:
	nlohmann::json to_json() const;

public:
	std::int32_t code{};
	std::string message{};
	nlohmann::json data{};
};

export struct jsonrpc_request
{
public:
	static jsonrpc_request from_json(const nlohmann::json& j);

public:
	nlohmann::json to_json() const;

public:
	std::int64_t id{};
	std::string jsonrpc{ "2.0" };

	std::string method{};
	nlohmann::json params{};
};

export struct jsonrpc_notification
{
public:
	static jsonrpc_notification from_json(const nlohmann::json& j);

public:
	nlohmann::json to_json() const;

public:
	std::string jsonrpc{ "2.0" };

	std::string method{};
	nlohmann::json params{};
};

export struct jsonrpc_response
{
public:
	static jsonrpc_response from_json(const nlohmann::json& j);

public:
	nlohmann::json to_json() const;

public:
	std::int64_t id{};
	std::string jsonrpc{ "2.0" };
	std::expected<nlohmann::json, jsonrpc_error> payload{};
};

export using jsonrpc_message = std::variant<jsonrpc_request, jsonrpc_notification, jsonrpc_response>;
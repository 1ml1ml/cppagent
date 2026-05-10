module;

#include <string>
#include <cstdint>
#include <variant>
#include <expected>

#include "nlohmann/json.hpp"

module jsonrpc;

nlohmann::json jsonrpc_id_to_json(const std::variant<std::int64_t, std::string>& id)
{
	return std::visit([](const auto& v) -> nlohmann::json { return v; }, id);
}

std::variant<std::int64_t, std::string> jsonrpc_id_from_json(const nlohmann::json& j)
{
	std::variant<std::int64_t, std::string> id{};
	if (j.is_number_integer())
	{
		id = j.get<std::int64_t>();
	}
	else if (j.is_string())
	{
		id = j.get<std::string>();
	}
	return id;
}

jsonrpc_error jsonrpc_error::from_json(const nlohmann::json& j)
{
	jsonrpc_error e{};
	e.code = j["code"];
	e.message = j["message"];
	e.data = j["data"];
	return e;
}

nlohmann::json jsonrpc_error::to_json() const
{
	nlohmann::json j{};
	j["code"] = code;
	j["message"] = message;
	j["data"] = data;
	return j;
}

jsonrpc_request jsonrpc_request::from_json(const nlohmann::json& j)
{
	jsonrpc_request req{};
	req.jsonrpc = j["jsonrpc"];
	req.id = jsonrpc_id_from_json(j["id"]);

	req.method = j["method"];
	req.params = j["params"];
	return req;
}

nlohmann::json jsonrpc_request::to_json() const
{
	nlohmann::json j{};
	j["jsonrpc"] = jsonrpc;
	j["id"] = jsonrpc_id_to_json(id);

	j["method"] = method;
	j["params"] = params;
	return j;
}

jsonrpc_notification jsonrpc_notification::from_json(const nlohmann::json& j)
{
	jsonrpc_notification n{};
	n.jsonrpc = j["jsonrpc"];
	n.method = j["method"];
	n.params = j["params"];
	return n;
}

nlohmann::json jsonrpc_notification::to_json() const
{
	nlohmann::json j{};
	j["jsonrpc"] = jsonrpc;
	j["method"] = method;
	j["params"] = params;
	return j;
}

jsonrpc_response jsonrpc_response::from_json(const nlohmann::json& j)
{
	jsonrpc_response res{};
	res.jsonrpc = j["jsonrpc"];
	res.id = jsonrpc_id_from_json(j["id"]);

	if (j.contains("error"))
	{
		res.payload = std::unexpected(jsonrpc_error::from_json(j["error"]));
	}
	else if (j.contains("result"))
	{
		res.payload = j["result"];
	}
	else
	{
		res.payload = std::unexpected(jsonrpc_error{ static_cast<std::int32_t>(error_code::INVALID_REQUEST), "Response must contain either 'result' or 'error'" });
	}

	return res;
}

nlohmann::json jsonrpc_response::to_json() const
{
	nlohmann::json j{};
	j["jsonrpc"] = jsonrpc;
	j["id"] = jsonrpc_id_to_json(id);

	if (payload)
	{
		j["result"] = payload.value();
	}
	else
	{
		j["error"] = payload.error().to_json();
	}

	return j;
}

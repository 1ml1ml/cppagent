module;

#include <string>
#include <cstdint>
#include <variant>
#include <expected>

#include "nlohmann/json.hpp"

module jsonrpc;

jsonrpc_error jsonrpc_error::from_json(const nlohmann::json& j)
{
	jsonrpc_error e{};
	e.code = j["code"];
	e.message = j["message"];
	e.data = j.value("data", nlohmann::json{});
	return e;
}

nlohmann::json jsonrpc_error::to_json() const
{
	nlohmann::json j{};
	j["code"] = code;
	j["message"] = message;

	if (!data.is_null())
	{
		j["data"] = data;
	}

	return j;
}

jsonrpc_request jsonrpc_request::from_json(const nlohmann::json& j)
{
	jsonrpc_request req{};
	req.jsonrpc = j["jsonrpc"];
	req.id = j["id"].get<std::int64_t>();

	req.method = j["method"];
	req.params = j.value("params", nlohmann::json{});
	return req;
}

nlohmann::json jsonrpc_request::to_json() const
{
	nlohmann::json j{};
	j["id"] = id;
	j["jsonrpc"] = jsonrpc;

	j["method"] = method;

	if (!params.is_null())
	{
		j["params"] = params;
	}

	return j;
}

jsonrpc_notification jsonrpc_notification::from_json(const nlohmann::json& j)
{
	jsonrpc_notification n{};
	n.jsonrpc = j["jsonrpc"];
	n.method = j["method"];
	n.params = j.value("params", nlohmann::json{});
	return n;
}

nlohmann::json jsonrpc_notification::to_json() const
{
	nlohmann::json j{};
	j["jsonrpc"] = jsonrpc;
	j["method"] = method;

	if (!params.is_null())
	{
		j["params"] = params;
	}

	return j;
}

jsonrpc_response jsonrpc_response::from_json(const nlohmann::json& j)
{
	jsonrpc_response res{};
	res.jsonrpc = j["jsonrpc"];
	res.id = j["id"].get<std::int64_t>();

	if (j.contains("error"))
	{
		res.payload = std::unexpected{ jsonrpc_error::from_json(j["error"]) };
	}
	else if (j.contains("result"))
	{
		res.payload = j["result"];
	}
	else
	{
		throw std::runtime_error{ "Response must contain either 'result' or 'error'" };
	}

	return res;
}

nlohmann::json jsonrpc_response::to_json() const
{
	nlohmann::json j{};
	j["id"] = id;
	j["jsonrpc"] = jsonrpc;

	if (payload)
	{
		if (payload->is_null())
		{
			throw std::runtime_error{ "Response must contain either 'result' or 'error'" };
		}

		j["result"] = payload.value();
	}
	else
	{
		j["error"] = payload.error().to_json();
	}

	return j;
}

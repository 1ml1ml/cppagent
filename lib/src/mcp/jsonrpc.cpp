module;

#include "nlohmann/json.hpp"

module mcp_jsonrpc;

namespace nlohmann {

void adl_serializer<mcp_jsonrpc::jsonrpc_error>::to_json(json& j, const mcp_jsonrpc::jsonrpc_error& e)
{
  j["code"] = e.code;
  j["message"] = e.message;

  if (!e.data.is_null())
  {
    j["data"] = e.data;
  }
}

void adl_serializer<mcp_jsonrpc::jsonrpc_error>::from_json(const json& j, mcp_jsonrpc::jsonrpc_error& e)
{
  e.code = j.value("code", std::int32_t{});
  e.message = j.value("message", std::string{});

  if (j.contains("data"))
  {
    e.data = j["data"];
  }
}

void adl_serializer<mcp_jsonrpc::jsonrpc_request>::to_json(json& j, const mcp_jsonrpc::jsonrpc_request& req)
{
  j["jsonrpc"] = req.jsonrpc;
  j["method"] = req.method;

  std::visit(
    [&j](auto&& val)
    {
      j["id"] = val;
    },
    req.id);

  if (!req.params.is_null() && !req.params.empty())
  {
    j["params"] = req.params;
  }
}

void adl_serializer<mcp_jsonrpc::jsonrpc_request>::from_json(const json& j, mcp_jsonrpc::jsonrpc_request& req)
{
  req.jsonrpc = j.value("jsonrpc", std::string{"2.0"});

  if (j.contains("id"))
  {
    if (j["id"].is_number_integer())
    {
      req.id = j["id"].get<std::int64_t>();
    }
    else if (j["id"].is_string())
    {
      req.id = j["id"].get<std::string>();
    }
  }

  req.method = j.value("method", std::string{});

  if (j.contains("params"))
  {
    req.params = j["params"];
  }
}

void adl_serializer<mcp_jsonrpc::jsonrpc_notification>::to_json(json& j, const mcp_jsonrpc::jsonrpc_notification& n)
{
  j["jsonrpc"] = n.jsonrpc;
  j["method"] = n.method;

  if (!n.params.is_null() && !n.params.empty())
  {
    j["params"] = n.params;
  }
}

void adl_serializer<mcp_jsonrpc::jsonrpc_notification>::from_json(const json& j, mcp_jsonrpc::jsonrpc_notification& n)
{
  n.jsonrpc = j.value("jsonrpc", std::string{"2.0"});
  n.method = j.value("method", std::string{});

  if (j.contains("params"))
  {
    n.params = j["params"];
  }
}

void adl_serializer<mcp_jsonrpc::jsonrpc_response>::to_json(json& j, const mcp_jsonrpc::jsonrpc_response& res)
{
  j["jsonrpc"] = res.jsonrpc;

  std::visit(
    [&j](auto&& val)
    {
      j["id"] = val;
    },
    res.id);

  if (res.err)
  {
    j["error"] = *res.err;
  }
  else
  {
    j["result"] = res.result;
  }
}

void adl_serializer<mcp_jsonrpc::jsonrpc_response>::from_json(const json& j, mcp_jsonrpc::jsonrpc_response& res)
{
  res.jsonrpc = j.value("jsonrpc", std::string{"2.0"});

  if (j.contains("id"))
  {
    if (j["id"].is_number_integer())
    {
      res.id = j["id"].get<std::int64_t>();
    }
    else if (j["id"].is_string())
    {
      res.id = j["id"].get<std::string>();
    }
  }

  if (j.contains("error"))
  {
    res.err = j["error"].get<mcp_jsonrpc::jsonrpc_error>();
  }
  else if (j.contains("result"))
  {
    res.result = j["result"];
  }
}

} // namespace nlohmann

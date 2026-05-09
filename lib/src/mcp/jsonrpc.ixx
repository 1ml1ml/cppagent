module;

#include <cstdint>
#include <optional>
#include <string>
#include <variant>

#include "nlohmann/json.hpp"

export module mcp_jsonrpc;

export namespace mcp_jsonrpc {

enum class error_code : std::int32_t
{
  PARSE_ERROR = -32700,
  INVALID_REQUEST = -32600,
  METHOD_NOT_FOUND = -32601,
  INVALID_PARAMS = -32602,
  INTERNAL_ERROR = -32603,
};

struct jsonrpc_error
{
  std::int32_t code{};
  std::string message{};
  nlohmann::json data{};
};

struct jsonrpc_request
{
  std::string jsonrpc{"2.0"};
  std::variant<std::int64_t, std::string> id{};
  std::string method{};
  nlohmann::json params{nlohmann::json::object()};
};

struct jsonrpc_notification
{
  std::string jsonrpc{"2.0"};
  std::string method{};
  nlohmann::json params{nlohmann::json::object()};
};

struct jsonrpc_response
{
  std::string jsonrpc{"2.0"};
  std::variant<std::int64_t, std::string> id{};
  nlohmann::json result{};
  std::optional<jsonrpc_error> err{};
};

} // namespace mcp_jsonrpc

// === nlohmann::json adl_serializer explicit specialization ===

namespace nlohmann {

template <>
struct adl_serializer<mcp_jsonrpc::jsonrpc_error>
{
  static void to_json(json& j, const mcp_jsonrpc::jsonrpc_error& e);
  static void from_json(const json& j, mcp_jsonrpc::jsonrpc_error& e);
};

template <>
struct adl_serializer<mcp_jsonrpc::jsonrpc_request>
{
  static void to_json(json& j, const mcp_jsonrpc::jsonrpc_request& req);
  static void from_json(const json& j, mcp_jsonrpc::jsonrpc_request& req);
};

template <>
struct adl_serializer<mcp_jsonrpc::jsonrpc_notification>
{
  static void to_json(json& j, const mcp_jsonrpc::jsonrpc_notification& n);
  static void from_json(const json& j, mcp_jsonrpc::jsonrpc_notification& n);
};

template <>
struct adl_serializer<mcp_jsonrpc::jsonrpc_response>
{
  static void to_json(json& j, const mcp_jsonrpc::jsonrpc_response& res);
  static void from_json(const json& j, mcp_jsonrpc::jsonrpc_response& res);
};

} // namespace nlohmann

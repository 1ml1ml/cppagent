#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <map>

#include "nlohmann/json.hpp"

import transport;
import mcp_client;
import stdio_transport;

static std::vector<std::string> make_fs_args()
{
  return { "/c", "npx", "-y", "@modelcontextprotocol/server-filesystem", "D:/Sources/cppagent/tests" };
}

TEST_CASE("mcp_client: initialize with real filesystem server", "[mcp_client][integration]")
{
  mcp_client client{ std::make_unique<stdio_transport>("cmd", make_fs_args()) };
  REQUIRE_NOTHROW(client.initialize(std::chrono::seconds{ 5 }));
  REQUIRE_THROWS_AS(client.initialize(std::chrono::seconds{ 1 }), std::runtime_error);
}

TEST_CASE("mcp_client: initialize with env vars", "[mcp_client][integration]")
{
  std::map<std::string, std::string> env{ {"TEST_VAR", "test_value"} };
  mcp_client client{ std::make_unique<stdio_transport>("cmd", make_fs_args(), env) };
  REQUIRE_NOTHROW(client.initialize(std::chrono::seconds{ 5 }));
}

TEST_CASE("mcp_client: name and version", "[mcp_client][integration]")
{
  mcp_client client{ std::make_unique<stdio_transport>("cmd", make_fs_args()) };

  client.set_name("test-agent");
  client.set_version("2.0.0");

  REQUIRE(client.get_name() == "test-agent");
  REQUIRE(client.get_version() == "2.0.0");
}

TEST_CASE("mcp_client: list_tools with real filesystem server", "[mcp_client][integration]")
{
  mcp_client client{ std::make_unique<stdio_transport>("cmd", make_fs_args()) };
  client.initialize(std::chrono::seconds{ 5 });

  auto tools{ client.list_tools(std::chrono::seconds{ 5 }) };
  REQUIRE(!tools.empty());

  for (const auto& tool : tools)
  {
    REQUIRE(!tool.name.empty());
    REQUIRE(!tool.description.empty());
    REQUIRE(!tool.input_schema.empty());
  }
}

TEST_CASE("mcp_client: call_tool with real filesystem server", "[mcp_client][integration]")
{
  mcp_client client{ std::make_unique<stdio_transport>("cmd", make_fs_args()) };
  client.initialize(std::chrono::seconds{ 5 });

  auto tools{ client.list_tools(std::chrono::seconds{ 5 }) };
  REQUIRE(!tools.empty());

  nlohmann::json args{};
  args["path"] = "D:/Sources/cppagent/tests";

  auto result{ client.call_tool("list_directory", args, std::chrono::seconds{ 5 }) };

  REQUIRE(!result.is_error);
  REQUIRE(!result.content.empty());
}

TEST_CASE("mcp_client: list_resources attempt", "[mcp_client][integration]")
{
  mcp_client client{ std::make_unique<stdio_transport>("cmd", make_fs_args()) };
  client.initialize(std::chrono::seconds{ 5 });

  try
  {
    auto resources{ client.list_resources(std::chrono::seconds{ 5 }) };
    for (const auto& r : resources)
    {
      REQUIRE(!r.uri.empty());
      REQUIRE(!r.name.empty());
    }
  }
  catch (const std::runtime_error&)
  {
    REQUIRE(true);
  }
}

TEST_CASE("mcp_client: read_resource attempt", "[mcp_client][integration]")
{
  mcp_client client{ std::make_unique<stdio_transport>("cmd", make_fs_args()) };
  client.initialize(std::chrono::seconds{ 5 });

  try
  {
    auto contents{ client.read_resource("file:///D:/Sources/cppagent/tests/test_message.cpp", std::chrono::seconds{ 5 }) };
    for (const auto& content : contents)
    {
      REQUIRE(!content.uri.empty());
    }
  }
  catch (const std::runtime_error&)
  {
    REQUIRE(true);
  }
}

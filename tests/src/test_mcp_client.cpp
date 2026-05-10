#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include "nlohmann/json.hpp"

import transport;
import mcp_client;
import stdio_transport;

TEST_CASE("mcp_client: initialize with real filesystem server", "[mcp_client][integration]")
{
  mcp_client client{ std::make_unique<stdio_transport>("cmd", std::vector<std::string>{"/c", "npx", "-y", "@modelcontextprotocol/server-filesystem", "D:/Sources/cppagent/tests"}) };
  REQUIRE_NOTHROW(client.initialize(nlohmann::json::object(), std::chrono::seconds{ 30 }));
  REQUIRE_THROWS_AS(client.initialize(nlohmann::json::object(), std::chrono::seconds{ 1 }), std::runtime_error);
}

TEST_CASE("mcp_client: list_tools with real filesystem server", "[mcp_client][integration]")
{
  mcp_client client{ std::make_unique<stdio_transport>("cmd", std::vector<std::string>{"/c", "npx", "-y", "@modelcontextprotocol/server-filesystem", "D:/Sources/cppagent/tests"}) };
  client.initialize(nlohmann::json::object(), std::chrono::seconds{ 30 });

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
  mcp_client client{ std::make_unique<stdio_transport>("cmd", std::vector<std::string>{"/c", "npx", "-y", "@modelcontextprotocol/server-filesystem", "D:/Sources/cppagent/tests"}) };
  client.initialize(nlohmann::json::object(), std::chrono::seconds{ 30 });

  auto tools{ client.list_tools(std::chrono::seconds{ 5 }) };
  REQUIRE(!tools.empty());

  // call list_directory tool
  nlohmann::json args{};
  args["path"] = "D:/Sources/cppagent/tests";

  auto result{ client.call_tool("list_directory", args, std::chrono::seconds{ 5 }) };

  REQUIRE(!result.is_error);
  REQUIRE(!result.content.empty());
}

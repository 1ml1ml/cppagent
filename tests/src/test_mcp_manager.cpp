#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

import mcp_manager;

static nlohmann::json make_test_config()
{
  nlohmann::json config{};
  config["mcpServers"]["filesystem"]["command"] = "cmd";
  config["mcpServers"]["filesystem"]["args"] = std::vector<std::string>{"/c", "npx", "-y", "@modelcontextprotocol/server-filesystem", "D:/Sources/cppagent/tests"};
  config["mcpServers"]["filesystem"]["roots"] = std::vector<std::string>{"D:/Sources/cppagent/tests"};
  return config;
}

TEST_CASE("mcp_manager: load and get_tools_schema", "[mcp_manager][integration]")
{
  mcp_manager manager{};
  manager.load(make_test_config());

  auto tools{ manager.get_tools_schema() };
  REQUIRE(tools.is_array());
  REQUIRE(!tools.empty());

  bool found_prefixed{ false };
  for (const auto& tool : tools)
  {
    REQUIRE(tool.contains("type"));
    REQUIRE(tool["type"] == "function");
    REQUIRE(tool.contains("function"));

    auto name{ tool["function"]["name"].get<std::string>() };
    REQUIRE(!name.empty());

    // 验证 server/tool 前缀格式
    if (name.find("filesystem/") == 0)
    {
      found_prefixed = true;
    }
  }

  REQUIRE(found_prefixed);
}

TEST_CASE("mcp_manager: call_tool with prefix", "[mcp_manager][integration]")
{
  mcp_manager manager{};
  manager.load(make_test_config());

  nlohmann::json args{};
  args["path"] = "D:/Sources/cppagent/tests";

  auto result{ manager.call_tool("filesystem/list_directory", args) };

  REQUIRE(result.contains("is_error"));
  REQUIRE(result["is_error"] == false);
  REQUIRE(result.contains("content"));
  REQUIRE(!result["content"].empty());
}

TEST_CASE("mcp_manager: call_tool invalid format", "[mcp_manager]")
{
  mcp_manager manager{};
  manager.load(make_test_config());

  REQUIRE_THROWS_AS(manager.call_tool("no_slash_name", nlohmann::json::object()), std::runtime_error);
}

TEST_CASE("mcp_manager: call_tool unknown server", "[mcp_manager]")
{
  mcp_manager manager{};
  manager.load(make_test_config());

  REQUIRE_THROWS_AS(manager.call_tool("unknown/tool", nlohmann::json::object()), std::runtime_error);
}

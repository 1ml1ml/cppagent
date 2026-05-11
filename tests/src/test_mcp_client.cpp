#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <map>

#include "nlohmann/json.hpp"

import transport;
import mcp_client;
import stdio_transport;

static std::vector<std::string> make_fs_args()
{
  return { "/c", "npx", "-y", "@modelcontextprotocol/server-filesystem", "D:/Sources/cppagent/tests" };
}

static nlohmann::json make_default_roots()
{
  nlohmann::json roots{ nlohmann::json::array() };
  roots.push_back({ {"uri", "file:///D:/Sources/cppagent/tests"}, {"name", "tests"} });
  return roots;
}

TEST_CASE("mcp_client: initialize with real filesystem server", "[mcp_client][integration]")
{
  mcp_client client{ std::make_unique<stdio_transport>("cmd", make_fs_args()) };
  client.set_roots(make_default_roots());
  REQUIRE_NOTHROW(client.initialize(std::chrono::seconds{ 30 }));
  REQUIRE_THROWS_AS(client.initialize(std::chrono::seconds{ 1 }), std::runtime_error);
}

TEST_CASE("mcp_client: initialize with env vars", "[mcp_client][integration]")
{
  std::map<std::string, std::string> env{ {"TEST_VAR", "test_value"} };
  mcp_client client{ std::make_unique<stdio_transport>("cmd", make_fs_args(), env) };
  client.set_roots(make_default_roots());
  REQUIRE_NOTHROW(client.initialize(std::chrono::seconds{ 30 }));
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
  client.set_roots(make_default_roots());
  client.initialize(std::chrono::seconds{ 30 });

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
  client.set_roots(make_default_roots());
  client.initialize(std::chrono::seconds{ 30 });

  auto tools{ client.list_tools(std::chrono::seconds{ 5 }) };
  REQUIRE(!tools.empty());

  nlohmann::json args{};
  args["path"] = "D:/Sources/cppagent/tests";

  auto result{ client.call_tool("list_directory", args, std::chrono::seconds{ 5 }) };

  REQUIRE(!result.is_error);
  REQUIRE(!result.content.empty());
}

TEST_CASE("mcp_client: roots get/set", "[mcp_client][integration]")
{
  mcp_client client{ std::make_unique<stdio_transport>("cmd", make_fs_args()) };
  client.set_roots(make_default_roots());
  client.initialize(std::chrono::seconds{ 30 });

  REQUIRE(client.get_roots().is_array());
  REQUIRE(client.get_roots().empty());

  nlohmann::json new_roots{ nlohmann::json::array() };
  new_roots.push_back({ {"uri", "file:///home/user/project"}, {"name", "project"} });

  client.set_roots(new_roots);
  REQUIRE(client.get_roots().size() == 1);
  REQUIRE(client.get_roots()[0]["uri"] == "file:///home/user/project");
}

TEST_CASE("mcp_client: roots change notification after initialized", "[mcp_client][integration]")
{
  mcp_client client{ std::make_unique<stdio_transport>("cmd", make_fs_args()) };
  client.set_roots(make_default_roots());
  client.initialize(std::chrono::seconds{ 30 });

  nlohmann::json roots1{ nlohmann::json::array() };
  roots1.push_back({ {"uri", "file:///a"}, {"name", "a"} });

  // 第一次设置应触发 list_changed notification（不抛异常即可）
  REQUIRE_NOTHROW(client.set_roots(roots1));

  // 相同值重复设置不应触发 notification
  REQUIRE_NOTHROW(client.set_roots(roots1));

  nlohmann::json roots2{ nlohmann::json::array() };
  roots2.push_back({ {"uri", "file:///b"}, {"name", "b"} });

  // 不同值应触发 notification
  REQUIRE_NOTHROW(client.set_roots(roots2));
}

TEST_CASE("mcp_client: set_roots rejects non-array", "[mcp_client]")
{
  mcp_client client{ std::make_unique<stdio_transport>("cmd", make_fs_args()) };

  nlohmann::json bad{ {"not", "array"} };
  REQUIRE_THROWS_AS(client.set_roots(bad), std::runtime_error);
}

TEST_CASE("mcp_client: list_resources attempt", "[mcp_client][integration]")
{
  mcp_client client{ std::make_unique<stdio_transport>("cmd", make_fs_args()) };
  client.set_roots(make_default_roots());
  client.initialize(std::chrono::seconds{ 30 });

  // filesystem server 可能不支持 resources，调用本身不崩溃即可
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
    // server 不支持 resources 是可接受的
    REQUIRE(true);
  }
}

TEST_CASE("mcp_client: read_resource attempt", "[mcp_client][integration]")
{
  mcp_client client{ std::make_unique<stdio_transport>("cmd", make_fs_args()) };
  client.set_roots(make_default_roots());
  client.initialize(std::chrono::seconds{ 30 });

  // 尝试读取一个 URI；filesystem server 通常不支持 resources
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
    // server 不支持 resources 是可接受的
    REQUIRE(true);
  }
}

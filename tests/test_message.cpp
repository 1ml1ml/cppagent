#include <catch2/catch_test_macros.hpp>

#include "nlohmann/json.hpp"

import message;

TEST_CASE("message role to string", "[message]")
{
  REQUIRE(std::string(message::role_to_string(message::role::user)) == "user");
  REQUIRE(std::string(message::role_to_string(message::role::assistant)) == "assistant");
}

TEST_CASE("user_message construction", "[message]")
{
  auto msg{ std::make_shared<user_message>("hello") };
  REQUIRE(msg->get_role() == message::role::user);
  REQUIRE(msg->content() == "hello");
  REQUIRE(msg->get_attachments_ref().empty());
  REQUIRE(msg->get_tool_call_results_ref().empty());
}

TEST_CASE("user_message default content", "[message]")
{
  auto msg{ std::make_shared<user_message>() };
  REQUIRE(msg->content().empty());
}

TEST_CASE("assistant_message construction", "[message]")
{
  auto msg{ std::make_shared<assistant_message>("world") };
  REQUIRE(msg->get_role() == message::role::assistant);
  REQUIRE(msg->content() == "world");
  REQUIRE(msg->get_tool_calls_ref().empty());
}

TEST_CASE("assistant_message default content", "[message]")
{
  auto msg{ std::make_shared<assistant_message>() };
  REQUIRE(msg->content().empty());
}

TEST_CASE("assistant_message with tool_calls", "[message]")
{
  auto msg{ std::make_shared<assistant_message>("call tool") };

  std::vector<tool_call> calls{};
  calls.emplace_back("id1", "filesystem/list_directory", nlohmann::json::parse(R"({"path":"D:/"})"));
  msg->set_tool_calls(calls);

  REQUIRE(msg->get_tool_calls_ref().size() == 1);
  REQUIRE(msg->get_tool_calls_ref()[0].id == "id1");
  REQUIRE(msg->get_tool_calls_ref()[0].tool_name == "filesystem/list_directory");
}

TEST_CASE("tool_call_result construction", "[message]")
{
  tool_call_result result{};
  result.tool_call_id = "tc_1";
  result.error = false;
  result.result = nlohmann::json::parse(R"({"files":["a.txt"]})");

  REQUIRE(result.tool_call_id == "tc_1");
  REQUIRE(result.error == false);
  REQUIRE(result.result["files"][0].get<std::string>() == "a.txt");
}

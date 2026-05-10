#include <catch2/catch_test_macros.hpp>
#include <memory>

import message;

using namespace std::string_view_literals;

TEST_CASE("message 默认构造", "[message]")
{
  auto msg{std::make_shared<message>()};
  REQUIRE(msg->get_role() == message::role::unknown);
  REQUIRE(msg->get_content().empty());
  REQUIRE(msg->attachments().empty());
}

TEST_CASE("message 带参数构造", "[message]")
{
  auto msg{std::make_shared<message>(message::role::user, "hello"sv)};
  REQUIRE(msg->get_role() == message::role::user);
  REQUIRE(msg->get_content() == "hello"sv);
}

TEST_CASE("message role 转换字符串", "[message]")
{
  REQUIRE(std::string_view(message::role_to_string(message::role::user)) == "user"sv);
  REQUIRE(std::string_view(message::role_to_string(message::role::system)) == "system"sv);
  REQUIRE(std::string_view(message::role_to_string(message::role::assistant)) == "assistant"sv);
  REQUIRE(std::string_view(message::role_to_string(message::role::unknown)) == "unknown"sv);
}

TEST_CASE("message 修改 content", "[message]")
{
  auto msg{std::make_shared<message>(message::role::user, "old"sv)};
  msg->set_content("new content"sv);
  REQUIRE(msg->get_content() == "new content"sv);
}

TEST_CASE("message 修改 role", "[message]")
{
  auto msg{std::make_shared<message>(message::role::user, "text"sv)};
  msg->set_role(message::role::assistant);
  REQUIRE(msg->get_role() == message::role::assistant);
}

TEST_CASE("message attachment 操作", "[message]")
{
  auto msg{std::make_shared<message>(message::role::user, "text"sv)};

  message::attachment att{};
  att.name = "test.txt";
  att.mime_type = "text/plain";
  att.data = {std::byte{'a'}, std::byte{'b'}};

  msg->attach(att);
  REQUIRE(msg->attachments().size() == 1);
  REQUIRE(msg->attachments()[0].name == "test.txt");

  msg->clear_attachments();
  REQUIRE(msg->attachments().empty());
}

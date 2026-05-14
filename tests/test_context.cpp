#include <catch2/catch_test_macros.hpp>

import context;
import message;

TEST_CASE("context empty", "[context]")
{
  auto ctx{ std::make_shared<context>() };
  REQUIRE(ctx->size() == 0);
  REQUIRE(ctx->get_instructions().empty());
}

TEST_CASE("context instructions", "[context]")
{
  auto ctx{ std::make_shared<context>() };
  ctx->set_instructions("you are a helpful assistant");
  REQUIRE(ctx->get_instructions() == "you are a helpful assistant");
}

TEST_CASE("context append single message", "[context]")
{
  auto ctx{ std::make_shared<context>() };
  ctx->append(std::make_shared<user_message>("hello"));

  REQUIRE(ctx->size() == 1);
  REQUIRE(ctx->messages_ref()[0]->get_content() == "hello");
  REQUIRE(ctx->messages_ref()[0]->get_role() == message::role::user);
}

TEST_CASE("context append multiple messages", "[context]")
{
  auto ctx{ std::make_shared<context>() };

  std::vector<message_shared_ptr> msgs{};
  msgs.push_back(std::make_shared<user_message>("q1"));
  msgs.push_back(std::make_shared<assistant_message>("a1"));
  msgs.push_back(std::make_shared<user_message>("q2"));

  ctx->append(msgs);

  REQUIRE(ctx->size() == 3);
  REQUIRE(ctx->messages_ref()[0]->get_content() == "q1");
  REQUIRE(ctx->messages_ref()[1]->get_content() == "a1");
  REQUIRE(ctx->messages_ref()[2]->get_content() == "q2");
}

TEST_CASE("context clear", "[context]")
{
  auto ctx{ std::make_shared<context>() };
  ctx->append(std::make_shared<user_message>("hello"));
  REQUIRE(ctx->size() == 1);

  ctx->clear();
  REQUIRE(ctx->size() == 0);
}

TEST_CASE("context mixed conversation", "[context]")
{
  auto ctx{ std::make_shared<context>() };
  ctx->set_instructions("system prompt");

  ctx->append(std::make_shared<user_message>("user input"));
  ctx->append(std::make_shared<assistant_message>("assistant reply"));

  REQUIRE(ctx->size() == 2);
  REQUIRE(ctx->messages_ref()[0]->get_role() == message::role::user);
  REQUIRE(ctx->messages_ref()[1]->get_role() == message::role::assistant);
}

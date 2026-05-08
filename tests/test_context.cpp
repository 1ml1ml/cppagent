#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <vector>

import message;
import context;

using namespace std::string_view_literals;

TEST_CASE("context 默认构造为空", "[context]")
{
  auto ctx{std::make_shared<context>()};
  REQUIRE(ctx->empty());
  REQUIRE(ctx->size() == 0);
}

TEST_CASE("context 追加单条消息", "[context]")
{
  auto ctx{std::make_shared<context>()};
  auto msg{std::make_shared<message>(message::role::user, "hello"sv)};

  ctx->append(msg);

  REQUIRE(ctx->size() == 1);
  REQUIRE(!ctx->empty());
  REQUIRE(ctx->last_message()->get_content() == "hello"sv);
  REQUIRE(ctx->last_message()->get_role() == message::role::user);
}

TEST_CASE("context 追加多条消息", "[context]")
{
  auto ctx{std::make_shared<context>()};
  auto msg1{std::make_shared<message>(message::role::user, "first"sv)};
  auto msg2{std::make_shared<message>(message::role::assistant, "second"sv)};

  ctx->append(std::vector{msg1, msg2});

  REQUIRE(ctx->size() == 2);
  REQUIRE(ctx->last_message()->get_content() == "second"sv);
  REQUIRE(ctx->last_message()->get_role() == message::role::assistant);
}

TEST_CASE("context 清空", "[context]")
{
  auto ctx{std::make_shared<context>()};
  ctx->append(std::make_shared<message>(message::role::user, "hi"sv));

  ctx->clear();

  REQUIRE(ctx->empty());
  REQUIRE(ctx->size() == 0);
}

TEST_CASE("context 合并", "[context]")
{
  auto ctx1{std::make_shared<context>()};
  auto ctx2{std::make_shared<context>()};

  ctx1->append(std::make_shared<message>(message::role::user, "a"sv));
  ctx2->append(std::make_shared<message>(message::role::assistant, "b"sv));

  ctx1->merge(ctx2);

  REQUIRE(ctx1->size() == 2);
  REQUIRE(ctx1->messages_ref()[0]->get_content() == "a"sv);
  REQUIRE(ctx1->messages_ref()[1]->get_content() == "b"sv);
}

TEST_CASE("context messages_ref 零拷贝", "[context]")
{
  auto ctx{std::make_shared<context>()};
  auto msg1{std::make_shared<message>(message::role::user, "one"sv)};
  auto msg2{std::make_shared<message>(message::role::system, "two"sv)};

  ctx->append(msg1);
  ctx->append(msg2);

  const auto& msgs = ctx->messages_ref();
  REQUIRE(msgs.size() == 2);
  REQUIRE(msgs[0]->get_content() == "one"sv);
  REQUIRE(msgs[1]->get_content() == "two"sv);
}

TEST_CASE("context find_last", "[context]")
{
  auto ctx{std::make_shared<context>()};
  ctx->append(std::make_shared<message>(message::role::user, "u1"sv));
  ctx->append(std::make_shared<message>(message::role::assistant, "a1"sv));
  ctx->append(std::make_shared<message>(message::role::user, "u2"sv));

  auto last_user = ctx->find_last(message::role::user);
  REQUIRE(last_user != nullptr);
  REQUIRE(last_user->get_content() == "u2"sv);

  auto last_system = ctx->find_last(message::role::system);
  REQUIRE(last_system == nullptr);
}

TEST_CASE("context 迭代器", "[context]")
{
  auto ctx{std::make_shared<context>()};
  ctx->append(std::make_shared<message>(message::role::user, "first"sv));
  ctx->append(std::make_shared<message>(message::role::assistant, "second"sv));

  int count = 0;
  for (const auto& msg : *ctx)
  {
    ++count;
    REQUIRE(msg != nullptr);
  }
  REQUIRE(count == 2);
}

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

import openai_client;
import context;
import message;

using namespace std::string_view_literals;

TEST_CASE("openai_client 默认构造", "[openai_client]")
{
  openai_client client{};
  (void)client;
}

TEST_CASE("openai_client generate 多个 system message 抛异常", "[openai_client]")
{
  openai_client client{};
  nlohmann::json config{};

  auto ctx{std::make_shared<context>()};
  ctx->append(std::make_shared<message>(message::role::system, "sys1"sv));
  ctx->append(std::make_shared<message>(message::role::system, "sys2"sv));
  ctx->append(std::make_shared<message>(message::role::user, "hello"sv));

  REQUIRE_THROWS_AS(client.generate(config, ctx), std::runtime_error);
}

TEST_CASE("openai_client generate system message 不在首位抛异常", "[openai_client]")
{
  openai_client client{};
  nlohmann::json config{};

  auto ctx{std::make_shared<context>()};
  ctx->append(std::make_shared<message>(message::role::user, "hello"sv));
  ctx->append(std::make_shared<message>(message::role::system, "sys"sv));

  REQUIRE_THROWS_AS(client.generate(config, ctx), std::runtime_error);
}

TEST_CASE("openai_client generate_async 异常透传", "[openai_client]")
{
  openai_client client{};
  nlohmann::json config{};

  auto ctx{std::make_shared<context>()};
  ctx->append(std::make_shared<message>(message::role::system, "s1"sv));
  ctx->append(std::make_shared<message>(message::role::system, "s2"sv));

  auto fut{client.generate_async(config, ctx)};
  REQUIRE_THROWS_AS(fut.get(), std::runtime_error);
}

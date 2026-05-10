#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include <memory>

import openai_provider;
import llm_provider;
import context;
import message;

using namespace std::string_view_literals;

TEST_CASE("openai_provider 默认构造", "[openai_provider]")
{
  openai_provider provider{};
  REQUIRE(provider.get_config().empty());
}

TEST_CASE("openai_provider config 读写", "[openai_provider]")
{
  openai_provider provider{};

  nlohmann::json cfg{};
  cfg["model"] = "moonshot-v1-8k";
  cfg["base_url"] = "https://api.moonshot.cn/v1";
  cfg["api_key"] = "sk-test123";
  cfg["temperature"] = 0.7;

  provider.set_config(cfg);

  auto result = provider.get_config();
  REQUIRE(result["model"].get<std::string>() == "moonshot-v1-8k");
  REQUIRE(result["base_url"].get<std::string>() == "https://api.moonshot.cn/v1");
  REQUIRE(result["api_key"].get<std::string>() == "sk-test123");
  REQUIRE(result["temperature"].get<float>() == 0.7f);
}

TEST_CASE("openai_provider generate 多个 system message 抛异常", "[openai_provider]")
{
  openai_provider provider{};
  nlohmann::json config{};
  config["model"] = "test";
  config["base_url"] = "https://test.com";
  config["api_key"] = "test-key";
  provider.set_config(config);

  auto ctx{std::make_shared<context>()};
  ctx->append(std::make_shared<message>(message::role::system, "sys1"sv));
  ctx->append(std::make_shared<message>(message::role::system, "sys2"sv));
  ctx->append(std::make_shared<message>(message::role::user, "hello"sv));

  REQUIRE_THROWS_AS(provider.generate(ctx), std::runtime_error);
}

TEST_CASE("openai_provider generate system message 不在首位抛异常", "[openai_provider]")
{
  openai_provider provider{};
  nlohmann::json config{};
  config["model"] = "test";
  config["base_url"] = "https://test.com";
  config["api_key"] = "test-key";
  provider.set_config(config);

  auto ctx{std::make_shared<context>()};
  ctx->append(std::make_shared<message>(message::role::user, "hello"sv));
  ctx->append(std::make_shared<message>(message::role::system, "sys"sv));

  REQUIRE_THROWS_AS(provider.generate(ctx), std::runtime_error);
}

TEST_CASE("openai_provider generate_async 异常透传", "[openai_provider]")
{
  openai_provider provider{};
  nlohmann::json config{};
  config["model"] = "test";
  config["base_url"] = "https://test.com";
  config["api_key"] = "test-key";
  provider.set_config(config);

  auto ctx{std::make_shared<context>()};
  ctx->append(std::make_shared<message>(message::role::system, "s1"sv));
  ctx->append(std::make_shared<message>(message::role::system, "s2"sv));

  auto fut{provider.generate_async(ctx)};
  REQUIRE_THROWS_AS(fut.get(), std::runtime_error);
}

TEST_CASE("openai_provider generate 无效网络配置抛异常", "[openai_provider]")
{
  openai_provider provider{};
  nlohmann::json config{};
  config["model"] = "test";
  config["base_url"] = "https://test.com";
  config["api_key"] = "test-key";
  provider.set_config(config);

  auto ctx{std::make_shared<context>()};
  ctx->append(std::make_shared<message>(message::role::user, "hi"sv));

  // 无效 base_url 导致网络请求失败
  REQUIRE_THROWS(provider.generate(ctx));
}

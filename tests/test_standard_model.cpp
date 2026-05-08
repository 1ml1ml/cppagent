#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include <memory>

import standard_model;

using namespace std::string_view_literals;

TEST_CASE("standard_model 默认构造", "[standard_model]")
{
  standard_model model{};

  REQUIRE(model.get_name().empty());
  REQUIRE(model.get_display_name().empty());
  REQUIRE(model.get_provider_name().empty());
  REQUIRE(model.get_base_url().empty());
  REQUIRE(model.get_api_key().empty());
  REQUIRE(model.get_client() == nullptr);
}

TEST_CASE("standard_model name 读写", "[standard_model]")
{
  standard_model model{};

  model.set_name("gpt-4"sv);
  REQUIRE(model.get_name() == "gpt-4");
}

TEST_CASE("standard_model display_name 读写", "[standard_model]")
{
  standard_model model{};

  model.set_display_name("GPT-4 Turbo"sv);
  REQUIRE(model.get_display_name() == "GPT-4 Turbo");
}

TEST_CASE("standard_model provider_name 读写", "[standard_model]")
{
  standard_model model{};

  model.set_provider_name("openai"sv);
  REQUIRE(model.get_provider_name() == "openai");
}

TEST_CASE("standard_model base_url 读写", "[standard_model]")
{
  standard_model model{};

  model.set_base_url("https://api.openai.com"sv);
  REQUIRE(model.get_base_url() == "https://api.openai.com");
}

TEST_CASE("standard_model api_key 读写", "[standard_model]")
{
  standard_model model{};

  model.set_api_key("sk-test123"sv);
  REQUIRE(model.get_api_key() == "sk-test123");
}

TEST_CASE("standard_model config 读写", "[standard_model]")
{
  standard_model model{};

  nlohmann::json cfg{};
  cfg["temperature"] = 0.7;

  model.set_config(cfg);
  auto result{model.get_config()};

  REQUIRE(result.contains("temperature"));
  REQUIRE(result["temperature"].get<double>() == 0.7);
}

TEST_CASE("standard_factory 创建模型", "[standard_model]")
{
  standard_factory factory{};
  REQUIRE(factory.name() == "standard"sv);

  auto model{factory.create()};
  REQUIRE(model != nullptr);
}


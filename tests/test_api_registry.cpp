#include <catch2/catch_test_macros.hpp>

import api_registry;
import llm_api;
import chat_completion_api;

TEST_CASE("api_registry singleton", "[api_registry]")
{
  auto& reg1 = api_registry::instance();
  auto& reg2 = api_registry::instance();
  REQUIRE(&reg1 == &reg2);
}

TEST_CASE("api_registry register and create", "[api_registry]")
{
  auto& reg = api_registry::instance();
  reg.clear();

  reg.register_factory("openai", std::make_shared<chat_completion_api_factory>());

  REQUIRE(reg.has_provider("openai") == true);
  REQUIRE(reg.has_provider("anthropic") == false);

  auto api = reg.create("openai");
  REQUIRE(api != nullptr);
}

TEST_CASE("api_registry provider names", "[api_registry]")
{
  auto& reg = api_registry::instance();
  reg.clear();

  reg.register_factory("openai", std::make_shared<chat_completion_api_factory>());
  reg.register_factory("anthropic", std::make_shared<chat_completion_api_factory>());

  auto names = reg.provider_names();
  REQUIRE(names.size() == 2);
}

TEST_CASE("api_registry unknown provider throws", "[api_registry]")
{
  auto& reg = api_registry::instance();
  reg.clear();

  REQUIRE_THROWS_AS(reg.create("nonexistent"), std::invalid_argument);
}

TEST_CASE("api_registry unregister", "[api_registry]")
{
  auto& reg = api_registry::instance();
  reg.clear();

  reg.register_factory("openai", std::make_shared<chat_completion_api_factory>());
  REQUIRE(reg.has_provider("openai") == true);

  reg.unregister("openai");
  REQUIRE(reg.has_provider("openai") == false);
}

TEST_CASE("api_registry clear", "[api_registry]")
{
  auto& reg = api_registry::instance();
  reg.clear();

  reg.register_factory("openai", std::make_shared<chat_completion_api_factory>());
  reg.register_factory("anthropic", std::make_shared<chat_completion_api_factory>());
  REQUIRE(reg.provider_names().size() == 2);

  reg.clear();
  REQUIRE(reg.provider_names().empty());
}

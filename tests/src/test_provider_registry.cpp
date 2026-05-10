#include <catch2/catch_test_macros.hpp>
#include <memory>

import provider_registry;
import llm_provider;

TEST_CASE("provider_registry 单例", "[provider_registry]")
{
  auto& reg1{provider_registry::instance()};
  auto& reg2{provider_registry::instance()};
  REQUIRE(&reg1 == &reg2);
}

TEST_CASE("provider_registry 初始为空", "[provider_registry]")
{
  auto& reg{provider_registry::instance()};
  reg.clear();

  REQUIRE(reg.provider_names().empty());
  REQUIRE(!reg.has_provider("openai"));
}

TEST_CASE("provider_registry 注册与创建", "[provider_registry]")
{
  auto& reg{provider_registry::instance()};
  reg.clear();

  // 注册一个 mock factory
  struct mock_factory : public llm_provider_factory
  {
    std::string_view name() const override { return "mock"; }
    provider_unique_ptr create() const override { return nullptr; }
  };

  reg.register_factory("mock", std::make_shared<mock_factory>());

  REQUIRE(reg.has_provider("mock"));
  REQUIRE(reg.provider_names().size() == 1);

  auto provider = reg.create("mock");
  REQUIRE(provider == nullptr); // mock factory 返回 nullptr
}

TEST_CASE("provider_registry 注销", "[provider_registry]")
{
  auto& reg{provider_registry::instance()};
  reg.clear();

  struct mock_factory : public llm_provider_factory
  {
    std::string_view name() const override { return "temp"; }
    provider_unique_ptr create() const override { return nullptr; }
  };

  reg.register_factory("temp", std::make_shared<mock_factory>());
  REQUIRE(reg.has_provider("temp"));

  reg.unregister("temp");
  REQUIRE(!reg.has_provider("temp"));
}

TEST_CASE("provider_registry 未知 provider 抛异常", "[provider_registry]")
{
  auto& reg{provider_registry::instance()};
  reg.clear();

  REQUIRE_THROWS_AS(reg.create("unknown"), std::invalid_argument);
}

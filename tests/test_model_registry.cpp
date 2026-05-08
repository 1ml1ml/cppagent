#include <catch2/catch_test_macros.hpp>
#include <memory>

import model_registry;
import i_model;

TEST_CASE("model_registry 单例", "[model_registry]")
{
  auto& reg1{model_registry::instance()};
  auto& reg2{model_registry::instance()};
  REQUIRE(&reg1 == &reg2);
}

TEST_CASE("model_registry 初始为空", "[model_registry]")
{
  auto& reg{model_registry::instance()};
  reg.clear();

  REQUIRE(reg.provider_names().empty());
  REQUIRE(!reg.has_provider("openai"));
}

TEST_CASE("model_registry 注册与创建", "[model_registry]")
{
  auto& reg{model_registry::instance()};
  reg.clear();

  // 注册一个 mock factory
  struct mock_factory : public i_model_factory
  {
    std::string_view name() const override { return "mock"; }
    model_shared_ptr create() const override { return nullptr; }
  };

  reg.register_factory("mock", std::make_shared<mock_factory>());

  REQUIRE(reg.has_provider("mock"));
  REQUIRE(reg.provider_names().size() == 1);

  auto model{reg.create("mock")};
  REQUIRE(model == nullptr); // mock factory 返回 nullptr
}

TEST_CASE("model_registry 注销", "[model_registry]")
{
  auto& reg{model_registry::instance()};
  reg.clear();

  struct mock_factory : public i_model_factory
  {
    std::string_view name() const override { return "temp"; }
    model_shared_ptr create() const override { return nullptr; }
  };

  reg.register_factory("temp", std::make_shared<mock_factory>());
  REQUIRE(reg.has_provider("temp"));

  reg.unregister("temp");
  REQUIRE(!reg.has_provider("temp"));
}


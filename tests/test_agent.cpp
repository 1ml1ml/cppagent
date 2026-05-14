#include <catch2/catch_test_macros.hpp>

#include "nlohmann/json.hpp"

import agent;
import api_registry;
import chat_completion_api;
import context;
import message;

TEST_CASE("agent default construction", "[agent]")
{
  auto a = std::make_shared<agent>();
  REQUIRE(a != nullptr);

  REQUIRE(a->get_model_config().empty());
  REQUIRE(a->get_mcp_manager() == nullptr);
}

TEST_CASE("agent set and get model_config", "[agent]")
{
  auto a = std::make_shared<agent>();

  nlohmann::json cfg;
  cfg["api"] = "openai";
  cfg["model"] = "moonshot-v1-128k";

  a->set_model_config(cfg);
  REQUIRE(a->get_model_config()["api"].get<std::string>() == "openai");
  REQUIRE(a->get_model_config()["model"].get<std::string>() == "moonshot-v1-128k");
}

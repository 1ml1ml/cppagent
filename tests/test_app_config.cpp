#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

import app_config;

TEST_CASE("app_config load and read", "[app_config]")
{
  auto temp_path{ std::filesystem::temp_directory_path() / "cppagent_test_config.json" };

  {
    std::ofstream f{ temp_path };
    f << R"({
      "model": {
        "api": "openai",
        "model": "moonshot-v1-128k",
        "base_url": "https://api.moonshot.cn",
        "api_key": "sk-test-key"
      },
      "mcpServers": {
        "test_server": {
          "command": "npx",
          "args": ["-y", "test-package"]
        }
      }
    })";
  }

  app_config cfg{};
  cfg.load(temp_path);

  auto model = cfg.model_config();
  REQUIRE(model["api"].get<std::string>() == "openai");
  REQUIRE(model["model"].get<std::string>() == "moonshot-v1-128k");
  REQUIRE(model["base_url"].get<std::string>() == "https://api.moonshot.cn");
  REQUIRE(model["api_key"].get<std::string>() == "sk-test-key");

  auto mcp = cfg.mcp_servers();
  REQUIRE(mcp.contains("test_server"));
  REQUIRE(mcp["test_server"]["command"].get<std::string>() == "npx");

  std::filesystem::remove(temp_path);
}

TEST_CASE("app_config missing file throws", "[app_config]")
{
  app_config cfg{};
  REQUIRE_THROWS(cfg.load("/nonexistent/path/config.json"));
}

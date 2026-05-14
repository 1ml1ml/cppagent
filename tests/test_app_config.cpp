#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

import app_config;

TEST_CASE("app_config load and read", "[app_config]")
{
  // Create a temporary config file
  auto temp_path{ std::filesystem::temp_directory_path() / "cppagent_test_config.json" };

  {
    std::ofstream f{ temp_path };
    f << R"({
      "api_provider": "openai",
      "model": "moonshot-v1-128k",
      "base_url": "https://api.moonshot.cn",
      "api_key": "sk-test-key",
      "instructions": "test assistant",
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

  REQUIRE(cfg.api_provider() == "openai");
  REQUIRE(cfg.model() == "moonshot-v1-128k");
  REQUIRE(cfg.base_url() == "https://api.moonshot.cn");
  REQUIRE(cfg.api_key() == "sk-test-key");
  REQUIRE(cfg.instructions() == "test assistant");

  auto mcp{ cfg.mcp_servers() };
  REQUIRE(mcp.contains("test_server"));
  REQUIRE(mcp["test_server"]["command"].get<std::string>() == "npx");

  // Cleanup
  std::filesystem::remove(temp_path);
}

TEST_CASE("app_config missing file throws", "[app_config]")
{
  app_config cfg{};
  REQUIRE_THROWS(cfg.load("/nonexistent/path/config.json"));
}

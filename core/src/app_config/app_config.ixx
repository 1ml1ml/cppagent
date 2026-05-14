module;

#include <memory>
#include <string>
#include <filesystem>

#include "nlohmann/json.hpp"

export module app_config;

export class app_config
{
public:
  app_config();
  ~app_config();

  void load(const std::filesystem::path& path);

  nlohmann::json model_config() const;
  nlohmann::json mcp_servers() const;

private:
  class impl;
  std::unique_ptr<impl> impl{};
};

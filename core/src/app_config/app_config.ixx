module;

#include <filesystem>
#include <memory>
#include <string>

#include "nlohmann/json.hpp"

export module app_config;

export class app_config
{
public:
  app_config();
  ~app_config();

  void load(const std::filesystem::path& path);

  std::string api_provider() const;
  std::string model() const;
  std::string base_url() const;
  std::string api_key() const;
  std::string instructions() const;
  nlohmann::json mcp_servers() const;

private:
  class impl;
  std::unique_ptr<impl> impl{};
};

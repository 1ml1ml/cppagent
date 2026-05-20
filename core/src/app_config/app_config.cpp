module;

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "nlohmann/json.hpp"

module app_config;

class app_config::impl
{
public:
  nlohmann::json config{};
};

app_config::app_config()
  : impl{ std::make_unique<class impl>() }
{
}

app_config::~app_config() = default;

void app_config::load(const std::filesystem::path& path)
{
  std::ifstream file{ path };
  if (!file.is_open())
  {
    throw std::runtime_error{ "failed to open config file: " + path.string() };
  }

  impl->config = nlohmann::json::parse(file);
}

nlohmann::json app_config::model_config() const
{
  return impl->config.at("model");
}

nlohmann::json app_config::mcp_servers() const
{
  return impl->config.at("mcpServers");
}

module;

#include <memory>
#include <string>

#include "nlohmann/json.hpp"

module standard_model;

class standard_model::impl
{
public:
  nlohmann::json config{};
  client_unique_ptr client{};
};

standard_model::standard_model() :
  impl{ std::make_unique<class impl>() }
{
}

nlohmann::json standard_model::get_config() const
{
	return impl->config;
}

void standard_model::set_config(const nlohmann::json& config)
{
  impl->config = config;
}

std::string standard_model::get_name() const
{
	return impl->config.value("model", "");
}

void standard_model::set_name(const std::string_view& name)
{
	impl->config["model"] = name;
}

std::string standard_model::get_display_name() const
{
	return impl->config.value("display_name", "");
}

void standard_model::set_display_name(const std::string_view& display_name)
{
	impl->config["display_name"] = display_name;
}

std::string standard_model::get_provider_name() const
{
	return impl->config.value("provider", "");
}

void standard_model::set_provider_name(const std::string_view& provider_name)
{
	impl->config["provider"] = provider_name;
}

std::string standard_model::get_base_url() const
{
	return impl->config.value("base_url", "");
}

void standard_model::set_base_url(const std::string_view& base_url)
{
	impl->config["base_url"] = base_url;
}

std::string standard_model::get_api_key() const
{
	return impl->config.value("api_key", "");
}

void standard_model::set_api_key(const std::string_view& api_key)
{
	impl->config["api_key"] = api_key;
}

void standard_model::set_client(client_unique_ptr client)
{
  impl->client = std::move(client);
}

i_client* standard_model::get_client() const
{
  return impl->client.get();
}

std::string_view standard_factory::name() const
{
  return "standard";
}

model_shared_ptr standard_factory::create() const
{
  return std::make_shared<standard_model>();
}

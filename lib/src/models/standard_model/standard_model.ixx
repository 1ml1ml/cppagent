module;

#include <memory>
#include <string>

#include "nlohmann/json.hpp"

export module standard_model;

import i_model;
import i_client;

export class standard_model final : public i_model
{
public:
  standard_model();

public:
  nlohmann::json get_config() const override;
  void set_config(const nlohmann::json& config) override;

  std::string get_name() const override;
  void set_name(const std::string_view& name) override;

  std::string get_display_name() const override;
  void set_display_name(const std::string_view& display_name) override;

  std::string get_provider_name() const override;
  void set_provider_name(const std::string_view& provider_name) override;

  std::string get_base_url() const override;
  void set_base_url(const std::string_view& base_url) override;

  std::string get_api_key() const override;
  void set_api_key(const std::string_view& api_key) override;

  void set_client(client_unique_ptr client) override;
  i_client* get_client() const override;

private:
  class impl;
  std::unique_ptr<impl> impl{};
};

export class standard_factory final : public i_model_factory
{
public:
  std::string_view name() const override;
  model_shared_ptr create() const override;
};

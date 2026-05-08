module;

#include <memory>
#include <string>

#include "nlohmann/json.hpp"

export module i_model;

import i_client;

class i_model;
export using model_shared_ptr = std::shared_ptr<i_model>;

export class i_model : public std::enable_shared_from_this<i_model>
{
public:
  virtual ~i_model() = default;

public:
	virtual nlohmann::json get_config() const = 0;
  virtual void set_config(const nlohmann::json& config) = 0;

  virtual std::string get_name() const = 0;
	virtual void set_name(const std::string_view& name) = 0;

  virtual std::string get_display_name() const = 0;
	virtual void set_display_name(const std::string_view& display_name) = 0;

  virtual std::string get_provider_name() const = 0;
	virtual void set_provider_name(const std::string_view& provider_name) = 0;

  virtual std::string get_base_url() const = 0;
	virtual void set_base_url(const std::string_view& base_url) = 0;

	virtual std::string get_api_key() const = 0;
	virtual void set_api_key(const std::string_view& api_key) = 0;

  virtual void set_client(client_unique_ptr client) = 0;
  virtual i_client* get_client() const = 0;
};

class i_model_factory;
export using model_factory_shared_ptr = std::shared_ptr<i_model_factory>;

export class i_model_factory : public std::enable_shared_from_this<i_model_factory>
{
public:
  virtual ~i_model_factory() = default;

public:
  virtual std::string_view name() const = 0;
  virtual model_shared_ptr create() const = 0;
};

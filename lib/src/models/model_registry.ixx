module;

#include <vector>
#include <memory>
#include <string>

export module model_registry;

import i_model;

export class model_registry final
{
public:
  static model_registry& instance();

private:
  model_registry();

public:
  model_shared_ptr create(const std::string& provider_name) const;

  void register_factory(const std::string& provider_name, const model_factory_shared_ptr& factory);
  void unregister(const std::string& provider_name);

  std::vector<std::string> provider_names() const;
  bool has_provider(const std::string& provider_name) const;

  void clear();

private:
  class impl;
  std::unique_ptr<impl> impl{};
};

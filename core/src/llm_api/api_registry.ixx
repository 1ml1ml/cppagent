module;

#include <memory>
#include <string>
#include <vector>

export module api_registry;

import core;

export class api_registry
{
public:
  static api_registry& instance();

private:
  api_registry();
  ~api_registry();

public:
  api_unique_ptr create(const std::string& provider_name) const;

  void register_factory(const std::string& provider_name, const api_factory_shared_ptr& factory);
  void unregister(const std::string& provider_name);

  std::vector<std::string> provider_names() const;
  bool has_provider(const std::string& provider_name) const;

  void clear();

private:
  class impl;
  std::unique_ptr<impl> impl{};
};

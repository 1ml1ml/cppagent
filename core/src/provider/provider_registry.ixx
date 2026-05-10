module;

#include <memory>
#include <string>
#include <vector>

export module provider_registry;

import llm_provider;

export class provider_registry
{
public:
  static provider_registry& instance();

private:
  provider_registry();
  ~provider_registry();

public:
  provider_unique_ptr create(const std::string& provider_name) const;

  void register_factory(const std::string& provider_name, const provider_factory_shared_ptr& factory);
  void unregister(const std::string& provider_name);

  std::vector<std::string> provider_names() const;
  bool has_provider(const std::string& provider_name) const;

  void clear();

private:
  class impl;
  std::unique_ptr<impl> impl{};
};

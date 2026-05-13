module;

#include <map>
#include <string>
#include <vector>
#include <stdexcept>
#include <shared_mutex>

module api_registry;

import llm_provider;

api_registry& api_registry::instance()
{
  static api_registry inst{};
  return inst;
}

class api_registry::impl
{
public:
  mutable std::shared_mutex mutex{};
  std::map<std::string, provider_factory_shared_ptr> factories{};
};

api_registry::api_registry()
  : impl{ std::make_unique<class impl>() }
{
}

api_registry::~api_registry() = default;

provider_unique_ptr api_registry::create(const std::string& provider_name) const
{
  std::shared_lock lock(impl->mutex);

  auto it{ impl->factories.find(provider_name) };
  if (it == impl->factories.end())
  {
    throw std::invalid_argument("unknown provider: " + provider_name);
  }
  return it->second->create();
}

void api_registry::register_factory(const std::string& provider_name, const provider_factory_shared_ptr& factory)
{
  std::unique_lock lock(impl->mutex);
  impl->factories[provider_name] = factory;
}

void api_registry::unregister(const std::string& provider_name)
{
  std::unique_lock lock(impl->mutex);
  impl->factories.erase(provider_name);
}

std::vector<std::string> api_registry::provider_names() const
{
  std::shared_lock lock(impl->mutex);

  std::vector<std::string> names{};
  names.reserve(impl->factories.size());
  for (const auto& [name, factory] : impl->factories)
  {
    names.push_back(name);
  }
  return names;
}

bool api_registry::has_provider(const std::string& provider_name) const
{
  std::shared_lock lock(impl->mutex);
  return impl->factories.contains(provider_name);
}

void api_registry::clear()
{
  std::unique_lock lock(impl->mutex);
  impl->factories.clear();
}

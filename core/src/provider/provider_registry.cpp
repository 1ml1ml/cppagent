module;

#include <map>
#include <string>
#include <vector>
#include <stdexcept>
#include <shared_mutex>

module provider_registry;

import llm_provider;

provider_registry& provider_registry::instance()
{
  static provider_registry inst{};
  return inst;
}

class provider_registry::impl
{
public:
  mutable std::shared_mutex mutex{};
  std::map<std::string, provider_factory_shared_ptr> factories{};
};

provider_registry::provider_registry()
  : impl{ std::make_unique<class impl>() }
{
}

provider_registry::~provider_registry() = default;

provider_unique_ptr provider_registry::create(const std::string& provider_name) const
{
  std::shared_lock lock(impl->mutex);

  auto it{ impl->factories.find(provider_name) };
  if (it == impl->factories.end())
  {
    throw std::invalid_argument("unknown provider: " + provider_name);
  }
  return it->second->create();
}

void provider_registry::register_factory(const std::string& provider_name, const provider_factory_shared_ptr& factory)
{
  std::unique_lock lock(impl->mutex);
  impl->factories[provider_name] = factory;
}

void provider_registry::unregister(const std::string& provider_name)
{
  std::unique_lock lock(impl->mutex);
  impl->factories.erase(provider_name);
}

std::vector<std::string> provider_registry::provider_names() const
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

bool provider_registry::has_provider(const std::string& provider_name) const
{
  std::shared_lock lock(impl->mutex);
  return impl->factories.contains(provider_name);
}

void provider_registry::clear()
{
  std::unique_lock lock(impl->mutex);
  impl->factories.clear();
}

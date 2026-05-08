module;

#include <map>
#include <vector>
#include <memory>
#include <string>

module model_registry;

model_registry& model_registry::instance()
{
  static model_registry inst{};
  return inst;
}

class model_registry::impl
{
public:
  std::map<std::string, model_factory_shared_ptr> factories{};
};

model_registry::model_registry() :
  impl{ std::make_unique<class impl>() }
{
}

model_shared_ptr model_registry::create(const std::string& provider_name) const
{
  auto it{ impl->factories.find(provider_name) };
  return it != impl->factories.end() ? it->second->create() : nullptr;
}

void model_registry::register_factory(const std::string& provider_name, const model_factory_shared_ptr& factory)
{
  impl->factories[provider_name] = factory;
}

void model_registry::unregister(const std::string& provider_name)
{
  impl->factories.erase(provider_name);
}

std::vector<std::string> model_registry::provider_names() const
{
  std::vector<std::string> names{};
  for (const auto& [name, factory] : impl->factories)
  {
    names.push_back(name);
  }
  return names;
}

bool model_registry::has_provider(const std::string& provider_name) const
{
  return impl->factories.find(provider_name) != impl->factories.end();
}

void model_registry::clear()
{
  impl->factories.clear();
}

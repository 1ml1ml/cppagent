module;

#include <memory>
#include <string>
#include <vector>
#include <filesystem>
#include <string_view>

export module skill_manager;

import core;

class skill_manager;
export using skill_manager_shared_ptr = std::shared_ptr<skill_manager>;

export class skill_manager
{
public:
  skill_manager();
  ~skill_manager();

public:
  void load(const std::filesystem::path& dir);

  std::vector<skill_info> skills() const;
  bool has_skill(const std::string_view& name) const;

  std::string catalog_text() const;
  std::string load_body(const std::string_view& name) const;

	std::vector<tool_shared_ptr> list_tools() const;

private:
  class impl;
  std::unique_ptr<impl> impl{};
};

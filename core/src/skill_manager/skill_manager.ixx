module;

#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

export module skill_manager;

import core;

export class skill_manager
{
public:
  skill_manager();
  ~skill_manager();

  void load(const std::filesystem::path& dir);

  std::vector<skill_info> skills() const;

  bool has_skill(const std::string_view& name) const;

  std::string catalog_text() const;

  std::string load_body(const std::string_view& name) const;

private:
  class impl;
  std::unique_ptr<impl> impl{};
};

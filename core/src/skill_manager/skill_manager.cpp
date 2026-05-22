module;

#include <chrono>
#include <vector>
#include <format>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>
#include <string_view>

#include "nlohmann/json.hpp"

module skill_manager;

import core;

class skill_load_tool : public tool
{
public:
	tool::result call(const nlohmann::json& arguments, const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 5000 }) const override;

public:
	skill_manager_shared_ptr skill_manager{};
};

tool::result skill_load_tool::call(const nlohmann::json& arguments, const std::chrono::milliseconds& timeout) const
{
	return { false, skill_manager->load_body(name) };
}

class skill_manager::impl
{
public:
	static std::string extract_yaml_value(std::string_view yaml, std::string_view key);
	static skill_info parse_skill_md(const std::filesystem::path& path);

public:
	std::vector<skill_info> skills{};
};

std::string skill_manager::impl::extract_yaml_value(std::string_view yaml, std::string_view key)
{
	auto pos{ yaml.find(key) };
	if (pos == std::string_view::npos)
	{
		return {};
	}

	pos += key.length();

	while (pos < yaml.length() && (yaml[pos] == ' ' || yaml[pos] == '\t' || yaml[pos] == ':'))
	{
		++pos;
	}

	auto end_pos{ yaml.find('\n', pos) };
	if (end_pos == std::string_view::npos)
	{
		end_pos = yaml.length();
	}

	auto value{ yaml.substr(pos, end_pos - pos) };

	while (!value.empty() && (value.front() == ' ' || value.front() == '\t'))
	{
		value.remove_prefix(1);
	}
	while (!value.empty() && (value.back() == ' ' || value.back() == '\t' || value.back() == '\r'))
	{
		value.remove_suffix(1);
	}

	return value.data();
}

skill_info skill_manager::impl::parse_skill_md(const std::filesystem::path& path)
{
	std::ifstream file{ path };
	if (!file.is_open())
	{
		return {};
	}

	std::stringstream buffer{};
	buffer << file.rdbuf();
	std::string content{ buffer.str() };

	skill_info skill{};
	skill.file_path = path.string();

	if (content.starts_with("---"))
	{
		auto end_pos{ content.find("---", 3) };
		if (end_pos != std::string::npos)
		{
			auto frontmatter{ std::string_view{ content }.substr(3, end_pos - 3) };
			skill.name = extract_yaml_value(frontmatter, "name");
			skill.description = extract_yaml_value(frontmatter, "description");
		}
	}

	return skill;
}

skill_manager::skill_manager()
	: impl{ std::make_unique<class impl>() }
{
}

skill_manager::~skill_manager() = default;

void skill_manager::load(const std::filesystem::path& dir)
{
	if (!std::filesystem::exists(dir))
	{
		return;
	}

	for (const auto& entry : std::filesystem::directory_iterator(dir))
	{
		if (!entry.is_directory())
		{
			continue;
		}

		auto md_path{ entry.path() / "SKILL.md" };
		if (!std::filesystem::exists(md_path))
		{
			continue;
		}

		auto skill{ impl::parse_skill_md(md_path) };
		auto dir_name{ entry.path().filename().string() };
		if (skill.name != dir_name)
		{
			throw std::runtime_error{ std::format("skill name mismatch: directory '{}' but frontmatter name is '{}'", dir_name, skill.name.empty() ? "<empty>" : skill.name) };
		}

		skill.file_path = md_path.string();
		impl->skills.push_back(skill);
	}
}

std::vector<skill_info> skill_manager::skills() const
{
	return impl->skills;
}

bool skill_manager::has_skill(const std::string_view& name) const
{
	for (const auto& skill : impl->skills)
	{
		if (skill.name == name)
		{
			return true;
		}
	}
	return false;
}

std::string skill_manager::catalog_text() const
{
	if (impl->skills.empty())
	{
		return {};
	}

	std::string catalog{ "## 可用技能\n\n" };
	for (const auto& s : impl->skills)
	{
		catalog += std::format("- `{}`: {}\n", s.name, s.description);
	}

	return catalog;
}

std::string skill_manager::load_body(const std::string_view& name) const
{
	std::string content{};
	if (auto it{ std::ranges::find_if(impl->skills, [&name](const skill_info& s) { return s.name == name; }) }; it != impl->skills.end())
	{
		if (std::ifstream file{ it->file_path }; file.is_open())
		{
			std::stringstream buffer{};
			buffer << file.rdbuf();

			content = buffer.str();
			if (content.starts_with("---"))
			{
				auto end_pos{ content.find("---", 3) };
				if (end_pos != std::string::npos)
				{
					auto body_start{ end_pos + 3 };
					while (body_start < content.length() && (content[body_start] == '\n' || content[body_start] == '\r'))
					{
						++body_start;
					}
					return content.substr(body_start);
				}
			}
		}
	}
	return content;
}

std::vector<tool_shared_ptr> skill_manager::list_tools() const
{
	return std::vector<tool_shared_ptr>();
}

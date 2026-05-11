module;

#include <vector>
#include <memory>
#include <string>
#include <cstddef>
#include <fstream>
#include <optional>
#include <iostream>
#include <filesystem>
#include <string_view>

#include "mimetypes.hpp"

module message;

const char* message::role_to_string(role r) noexcept
{
	switch (r)
	{
	case role::system: return "system";
	case role::user: return "user";
	case role::assistant: return "assistant";
	case role::tool: return "tool";
	default: return "unknown";
	}
}

std::optional<message::attachment> message::attachment::from_file(const std::filesystem::path& path)
{
	if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path))
	{
		attachment att{};
		att.name = path.filename().string();
		att.mime_type = mimetypes::from_filename(path);

		if (auto size{ std::filesystem::file_size(path) }; size > 0)
		{
			if (std::ifstream file{ path, std::ios::binary }; file.is_open())
			{
				att.data.resize(size);
				file.read(reinterpret_cast<char*>(att.data.data()), size);

				if (file.good() && file.eof())
				{
					return att;
				}
			}
		}
	}

	return std::nullopt;
}

class message::impl
{
public:
	role role{ role::unknown };

	std::string content{};
	std::vector<attachment> attachments{};
};

message::message() :
	impl{ std::make_unique<class impl>() }
{
}

message::message(const role& r, const std::string_view& content, const std::vector<attachment>& attachments) : message()
{
	set_role(r);
	set_content(content);

	attach(attachments);
}

message::~message() = default;

void message::set_role(const role& r)
{
	impl->role = r;
}

void message::set_content(const std::string_view& content)
{
	impl->content = content;
}

message::role message::get_role() const
{
	return impl->role;
}

std::string_view message::get_content() const
{
	return impl->content;
}

void message::attach(const attachment& att)
{
	impl->attachments.push_back(att);
}

void message::attach(const std::vector<attachment>& attachments)
{
	impl->attachments.insert(impl->attachments.end(), attachments.begin(), attachments.end());
}

const std::vector<message::attachment>& message::attachments() const
{
	return impl->attachments;
}

void message::clear_attachments()
{
	impl->attachments.clear();
}

std::ostream& operator<<(std::ostream& os, message::role r)
{
	return os << message::role_to_string(r);
}

std::ostream& operator<<(std::ostream& os, const message& msg)
{
	os << "[" << msg.get_role() << "] ";

	if (auto content = msg.get_content(); !content.empty())
	{
		os << '"' << content << '"';
	}
	else
	{
		os << "(empty)";
	}

	if (auto atts = msg.attachments(); !atts.empty())
	{
		os << " [attachments: " << atts.size() << "]";
	}

	return os;
}

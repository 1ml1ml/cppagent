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
	case role::user: return "user";
	case role::tool: return "tool";
	case role::assistant: return "assistant";
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
	impl->role = r;
	impl->content = content;
	impl->attachments = attachments;
}

message::~message() = default;

message::role message::get_role() const
{
	return impl->role;
}

std::string_view message::get_content() const
{
	return impl->content;
}

std::vector<message::attachment>& message::attachments_ref() const
{
	return impl->attachments;
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

	if (auto atts = msg.attachments_ref(); !atts.empty())
	{
		os << " [attachments_ref: " << atts.size() << "]";
	}

	return os;
}

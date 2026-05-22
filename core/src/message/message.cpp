module;

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <cstddef>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <filesystem>
#include <string_view>

#include "mimetypes.hpp"

module message;

import core;

const char* message::role_to_string(enum class role r) noexcept
{
	switch (r)
	{
	case role::user: return "user";
	case role::assistant: return "assistant";
	}
	throw std::runtime_error{ "unknow role" };
}

class message::impl
{
public:
	enum class role role{ role::user };
	std::string content{};
};

message::message(const enum class role& r, const std::string_view& content) :
	impl{ std::make_unique<class impl>() }
{
	impl->role = r;
	impl->content = content;
}

message::~message() = default;

void message::accept(const visitor_shared_ptr& visitor)
{
	throw std::runtime_error{ "empty accept" };
}

enum class message::role message::role() const
{
	return impl->role;
}

std::string_view message::content() const
{
	return impl->content;
}

class user_message::impl
{
public:
	std::map<std::string, attachment> attachments{};
	std::vector<tool_call_result> tool_call_results{};
};

std::optional<user_message::attachment> user_message::attachment::from_file(const std::filesystem::path& path)
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

user_message::user_message(const std::string_view& content) : message(message::role::user, content),
impl{ std::make_unique<class impl>() }
{
}

user_message::~user_message() = default;

void user_message::accept(const visitor_shared_ptr& visitor)
{
	visitor->visit_user_message(std::static_pointer_cast<user_message>(shared_from_this()));
}

const std::map<std::string, user_message::attachment>& user_message::get_attachments_ref() const
{
	return impl->attachments;
}

void user_message::set_attachments(const std::map<std::string, attachment>& attachments)
{
	impl->attachments = attachments;
}

const std::vector<tool_call_result>& user_message::get_tool_call_results_ref() const
{
	return impl->tool_call_results;
}

void user_message::set_tool_call_results(const std::vector<tool_call_result>& results)
{
	impl->tool_call_results = results;
}

class assistant_message::impl
{
public:
	std::vector<tool_call> tool_calls{};
};

assistant_message::assistant_message(const std::string_view& content) : message(message::role::assistant, content),
impl{ std::make_unique<class impl>() }
{
}

assistant_message::~assistant_message() = default;

void assistant_message::accept(const visitor_shared_ptr& visitor)
{
	visitor->visit_assistant_message(std::static_pointer_cast<assistant_message>(shared_from_this()));
}

const std::vector<tool_call>& assistant_message::get_tool_calls_ref() const
{
	return impl->tool_calls;
}

void assistant_message::set_tool_calls(const std::vector<tool_call>& calls)
{
	impl->tool_calls = calls;
}

void message_visitor::visit_user_message(const std::shared_ptr<user_message>& message)
{
	throw std::runtime_error{ "empty visit" };
}

void message_visitor::visit_assistant_message(const std::shared_ptr<assistant_message>& message)
{
	throw std::runtime_error{ "empty visit" };
}

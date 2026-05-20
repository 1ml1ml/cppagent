module;

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <cstddef>
#include <optional>
#include <filesystem>
#include <string_view>

#include "nlohmann/json.hpp"

export module message;

import core;

export class message : public std::enable_shared_from_this<message>
{
public:
	enum class role
	{
		user,
		assistant,
	};
	static const char* role_to_string(role r) noexcept;

public:
	message(const role& r, const std::string_view& content);
	virtual ~message();

public:
	virtual void accept(const visitor_shared_ptr& visitor);

public:
	role get_role() const;
	std::string_view get_content() const;

private:
	class impl;
	std::unique_ptr<impl> impl{};
};

export class user_message : public message
{
public:
	struct attachment
	{
	public:
		static std::optional<attachment> from_file(const std::filesystem::path& path);

	public:
		std::string name{};
		std::string mime_type{};
		std::vector<std::byte> data{};
	};

public:
	user_message(const std::string_view& content = {});
	~user_message();

public:
	void accept(const visitor_shared_ptr& visitor) override;

public:
	const std::map<std::string, attachment>& get_attachments_ref() const;
	void set_attachments(const std::map<std::string, attachment>& attachments);

	const std::vector<tool_call_result>& get_tool_call_results_ref() const;
	void set_tool_call_results(const std::vector<tool_call_result>& results);

private:
	class impl;
	std::unique_ptr<impl> impl{};
};

export class assistant_message : public message
{
public:
	assistant_message(const std::string_view& content = {});
	~assistant_message();

public:
	void accept(const visitor_shared_ptr& visitor) override;

public:
	const std::vector<tool_call>& get_tool_calls_ref() const;
	void set_tool_calls(const std::vector<tool_call>& calls);

private:
	class impl;
	std::unique_ptr<impl> impl{};
};

export class message_visitor : public std::enable_shared_from_this<message_visitor>
{
public:
	virtual ~message_visitor() = default;

public:
	virtual void visit_user_message(const std::shared_ptr<user_message>& message);
	virtual void visit_assistant_message(const std::shared_ptr<assistant_message>& message);
};

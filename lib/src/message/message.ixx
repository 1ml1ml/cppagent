module;

#include <vector>
#include <memory>
#include <string>
#include <cstddef>
#include <optional>
#include <iostream>
#include <filesystem>
#include <string_view>

export module message;

class message;
export using message_shared_ptr = std::shared_ptr<message>;

export class message final : public std::enable_shared_from_this<message>
{
public:
	enum class role
	{
		unknown,
		user,
		system,
		assistant,
	};
	static const char* role_to_string(role r) noexcept;

	struct attachment final
	{
	public:
		static std::optional<attachment> from_file(const std::filesystem::path& path);

	public:
		std::string name{};
		std::string mime_type{};
		std::vector<std::byte> data{};
	};

public:
	message();
	message(const role& r, const std::string_view& content, const std::vector<attachment>& attachments = {});

	~message();

public:
	void set_role(const role& r);
	void set_content(const std::string_view& content);

	role get_role() const;
	std::string_view get_content() const;

	void attach(const attachment& att);
	void attach(const std::vector<attachment>& attachments);

	const std::vector<attachment>& attachments() const;

	void clear_attachments();

private:
	class impl;
	std::unique_ptr<impl> impl{};
};

export std::ostream& operator<<(std::ostream& os, message::role r);
export std::ostream& operator<<(std::ostream& os, const message& msg);

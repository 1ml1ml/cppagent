module;

#include <vector>
#include <memory>
#include <iostream>

module context;

import message;

class context::impl
{
public:
	std::vector<message_shared_ptr> msgs{};
};

context::context() :
	impl{ std::make_unique<class impl>() }
{
}

context::~context() = default;

void context::merge(const context_shared_ptr& ctx)
{
	append(ctx->messages_ref());
}

void context::append(const message_shared_ptr& msg)
{
	impl->msgs.push_back(msg);
}

void context::append(const std::vector<message_shared_ptr>& msgs)
{
	impl->msgs.insert(impl->msgs.end(), msgs.cbegin(), msgs.cend());
}

std::vector<message_shared_ptr> context::messages() const
{
	return impl->msgs;
}

const std::vector<message_shared_ptr>& context::messages_ref() const
{
	return impl->msgs;
}

size_t context::size() const
{
	return impl->msgs.size();
}

void context::clear()
{
	impl->msgs.clear();
}

std::ostream& operator<<(std::ostream& os, const context& ctx)
{
	if (ctx.size() == 0)
	{
		return os << "<empty context>";
	}

	os << "--- Context (" << ctx.size() << " messages) ---\n";
	for (const auto& msg : ctx.messages_ref())
	{
		if (msg)
		{
			os << "  " << *msg << '\n';
		}
	}
	os << "--- End Context ---";

	return os;
}
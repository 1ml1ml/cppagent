module;

#include <memory>
#include <string>
#include <vector>
#include <string_view>

module context;

import message;

class context::impl
{
public:
  std::string instructions{};
  std::vector<message_shared_ptr> msgs{};
};

context::context()
  : impl{ std::make_unique<class impl>() }
{
}

context::~context() = default;

std::string_view context::get_instructions() const
{
  return impl->instructions;
}

void context::set_instructions(const std::string_view& instructions)
{
  impl->instructions = instructions;
}

void context::append(const message_shared_ptr& msg)
{
  impl->msgs.push_back(msg);
}

void context::append(const std::vector<message_shared_ptr>& msgs)
{
  impl->msgs.insert(impl->msgs.end(), msgs.cbegin(), msgs.cend());
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
    }
  }
  os << "--- End Context ---";

  return os;
}

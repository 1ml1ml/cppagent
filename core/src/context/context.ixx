module;

#include <vector>
#include <memory>
#include <iostream>

export module context;

import message;

class context;
export using context_shared_ptr = std::shared_ptr<context>;

export class context : public std::enable_shared_from_this<context>
{
public:
  context();
  ~context();

public:
  std::string_view get_instructions() const;
  void set_instructions(const std::string_view& instructions);
  
  void append(const message_shared_ptr& msg);
  void append(const std::vector<message_shared_ptr>& msgs);

  const std::vector<message_shared_ptr>& messages_ref() const;

  size_t size() const;
  void clear();

private:
  class impl;
  std::unique_ptr<impl> impl{};
};

export std::ostream& operator<<(std::ostream& os, const context& ctx);

module;

#include <vector>
#include <memory>
#include <iostream>

export module context;

import message;

class context;
export using context_shared_ptr = std::shared_ptr<context>;

export class context final : public std::enable_shared_from_this<context>
{
public:
    context();
    ~context();

public:
    void merge(const context_shared_ptr& ctx);

    void append(const message_shared_ptr& msg);
    void append(const std::vector<message_shared_ptr>& msgs);

    message_shared_ptr last_message() const;
    std::vector<message_shared_ptr> messages() const;
    const std::vector<message_shared_ptr>& messages_ref() const;

    message_shared_ptr find_last(message::role r) const;

    bool empty() const;
    size_t size() const;

    void clear();

    auto begin() const;
    auto end() const;

private:
    class impl;
    std::unique_ptr<impl> impl{};
};

export std::ostream& operator<<(std::ostream& os, const context& ctx);
module;

#include <memory>
#include <future>
#include <string>
#include <functional>

#include "nlohmann/json.hpp"

export module i_client;

import context;

class i_client;
export using client_unique_ptr = std::unique_ptr<i_client>;

export class i_client : public std::enable_shared_from_this<i_client>
{
public:
    virtual ~i_client() = default;

public:
  virtual context_shared_ptr generate(const nlohmann::json& config, const context_shared_ptr& ctx, const std::function<bool(std::string data)>& stream_callback = {}) = 0;
    virtual std::future<context_shared_ptr> generate_async(const nlohmann::json& config, const context_shared_ptr& ctx, const std::function<bool(std::string data)>& stream_callback = {}) = 0;
};

module;

#include <memory>
#include <future>
#include <string>
#include <functional>

#include "nlohmann/json.hpp"

export module llm_provider;

import context;
import generation_result;

class llm_provider;
export using provider_unique_ptr = std::unique_ptr<llm_provider>;

export using stream_callback = std::function<bool(std::string data)>;

export class llm_provider {
public:
    virtual ~llm_provider() = default;

    // 配置管理
    virtual void set_config(const nlohmann::json& config) = 0;
    virtual nlohmann::json get_config() const = 0;

    // 通信
    virtual generation_result generate(
        const context_shared_ptr& ctx,
        const stream_callback& callback = {}) = 0;

    virtual std::future<generation_result> generate_async(
        const context_shared_ptr& ctx,
        const stream_callback& callback = {}) = 0;
};

// provider 工厂接口
class llm_provider_factory;
export using provider_factory_shared_ptr = std::shared_ptr<llm_provider_factory>;

export class llm_provider_factory : public std::enable_shared_from_this<llm_provider_factory> {
public:
    virtual ~llm_provider_factory() = default;

    virtual std::string_view name() const = 0;
    virtual provider_unique_ptr create() const = 0;
};

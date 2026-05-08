# P0/P1 问题修复方案

## 问题 1: generate() 返回结构丢失元数据

**当前**：
```cpp
context_shared_ptr generate(const nlohmann::json& config, const context_shared_ptr& ctx, ...);
```

**修复**：引入 `generation_result` 结构

```cpp
// lib/src/core/generation_result.ixx
export struct generation_result {
    message_shared_ptr message;                    // assistant 回复
    std::optional<std::string> finish_reason;      // stop/length/tool_calls
    std::optional<usage_info> usage;               // token 消耗
    std::string model;                             // 实际使用的模型
    std::string id;                                // 响应 ID
};

export struct usage_info {
    int prompt_tokens = 0;
    int completion_tokens = 0;
    int total_tokens = 0;
};

// i_client.ixx 修改接口
export class i_client {
public:
    virtual generation_result generate(
        const nlohmann::json& config, 
        const context_shared_ptr& ctx,
        const stream_callback& callback = {}) = 0;
};
```

**openai_client.cpp 修改**：
```cpp
generation_result openai_client::generate(...) {
    // ... 调用 API ...
    auto response = oai.ChatCompletion->create(...);
    
    generation_result result;
    result.message = std::make_shared<message>(
        message::role::assistant, 
        conv->GetLastResponse()
    );
    result.id = response["id"].get<std::string>();
    result.model = response["model"].get<std::string>();
    
    if (response.contains("usage")) {
        result.usage = usage_info{
            .prompt_tokens = response["usage"]["prompt_tokens"].get<int>(),
            .completion_tokens = response["usage"]["completion_tokens"].get<int>(),
            .total_tokens = response["usage"]["total_tokens"].get<int>()
        };
    }
    
    if (response.contains("choices") && !response["choices"].empty()) {
        result.finish_reason = response["choices"][0]["finish_reason"].get<std::string>();
    }
    
    return result;
}
```

---

## 问题 2: model 和 client 双向耦合

**当前**：model 持有 client
```cpp
class i_model {
    virtual void set_client(client_unique_ptr client) = 0;
    virtual i_client* get_client() const = 0;
};
```

**修复**：model 纯配置，client 纯通信，agent 负责组装

```cpp
// i_model.ixx — 去掉 client 相关
export class i_model : public std::enable_shared_from_this<i_model> {
public:
    virtual ~i_model() = default;
    
    virtual nlohmann::json get_config() const = 0;
    virtual void set_config(const nlohmann::json& config) = 0;
    
    virtual std::string get_name() const = 0;
    virtual void set_name(std::string_view name) = 0;
    
    virtual std::string get_base_url() const = 0;
    virtual void set_base_url(std::string_view url) = 0;
    
    virtual std::string get_api_key() const = 0;
    virtual void set_api_key(std::string_view key) = 0;
    
    // 新增：获取用于 client 的合并配置
    virtual nlohmann::json get_client_config() const = 0;
};

// standard_model.cpp
nlohmann::json standard_model::get_client_config() const {
    nlohmann::json client_cfg;
    client_cfg["model"] = get_name();
    client_cfg["base_url"] = get_base_url();
    client_cfg["api_key"] = get_api_key();
    
    // 从完整 config 复制其他参数
    auto full_cfg = get_config();
    for (auto& [key, val] : full_cfg.items()) {
        if (!client_cfg.contains(key)) {
            client_cfg[key] = val;
        }
    }
    return client_cfg;
}
```

---

## 问题 3: config 字段缺失直接抛 json 异常

**修复**：统一校验 + 明确异常

```cpp
// lib/src/core/config_validator.ixx
export class config_validation_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

export void validate_model_config(const nlohmann::json& config) {
    static const std::vector<std::string> required_fields{
        "model", "base_url", "api_key"
    };
    
    std::vector<std::string> missing;
    for (const auto& field : required_fields) {
        if (!config.contains(field) || config[field].is_null()) {
            missing.push_back(field);
        }
    }
    
    if (!missing.empty()) {
        std::string msg = "config 缺少必填字段: ";
        for (size_t i = 0; i < missing.size(); ++i) {
            if (i > 0) msg += ", ";
            msg += missing[i];
        }
        throw config_validation_error(msg);
    }
    
    // 类型校验
    if (!config["model"].is_string()) {
        throw config_validation_error("config['model'] 必须是字符串");
    }
    if (!config["base_url"].is_string()) {
        throw config_validation_error("config['base_url'] 必须是字符串");
    }
    if (!config["api_key"].is_string()) {
        throw config_validation_error("config['api_key'] 必须是字符串");
    }
}

// openai_client.cpp 中使用
context_shared_ptr openai_client::generate(const nlohmann::json& config, ...) {
    validate_model_config(config);  // 先校验
    
    // 现在可以安全访问
    liboai::OpenAI oai{ config["base_url"].get<std::string>() };
    oai.auth.SetKey(config["api_key"].get<std::string>());
    // ...
}
```

---

## 问题 4: context::messages() 返回值拷贝

**修复**：提供 const 引用版本 + 迭代器

```cpp
// context.ixx
export class context {
public:
    // 现有：返回副本（保持兼容性）
    std::vector<message_shared_ptr> messages() const;
    
    // 新增：返回 const 引用（零拷贝）
    const std::vector<message_shared_ptr>& messages_ref() const;
    
    // 新增：迭代器（更灵活）
    auto begin() const { return impl->msgs.begin(); }
    auto end() const { return impl->msgs.end(); }
    
    // 新增：消息过滤查询
    message_shared_ptr find_last(message::role r) const;
    std::vector<message_shared_ptr> filter(message::role r) const;
};

// context.cpp
const std::vector<message_shared_ptr>& context::messages_ref() const {
    return impl->msgs;  // ✅ 零拷贝
}

message_shared_ptr context::find_last(message::role r) const {
    for (auto it = impl->msgs.rbegin(); it != impl->msgs.rend(); ++it) {
        if ((*it)->get_role() == r) return *it;
    }
    return nullptr;
}
```

---

## 问题 5: model_registry 线程安全

**修复**：加读写锁

```cpp
// model_registry.cpp
#include <shared_mutex>

class model_registry::impl {
public:
    std::map<std::string, model_factory_shared_ptr> factories{};
    mutable std::shared_mutex mutex{};  // C++17 shared_mutex
};

model_shared_ptr model_registry::create(const std::string& provider_name) const {
    std::shared_lock lock(impl->mutex);  // 读锁
    auto it = impl->factories.find(provider_name);
    if (it == impl->factories.end()) {
        throw std::invalid_argument("unknown provider: " + provider_name);
    }
    return it->second->create();
}

void model_registry::register_factory(const std::string& provider_name, 
                                       const model_factory_shared_ptr& factory) {
    std::unique_lock lock(impl->mutex);  // 写锁
    impl->factories[provider_name] = factory;
}

void model_registry::unregister(const std::string& provider_name) {
    std::unique_lock lock(impl->mutex);
    impl->factories.erase(provider_name);
}

std::vector<std::string> model_registry::provider_names() const {
    std::shared_lock lock(impl->mutex);
    std::vector<std::string> names;
    names.reserve(impl->factories.size());
    for (const auto& [name, _] : impl->factories) {
        names.push_back(name);
    }
    return names;
}
```

---

## 问题 6: generate_async 假异步

**修复**：基于 future + 取消令牌

```cpp
// i_client.ixx
export struct cancel_token {
    std::atomic<bool> cancelled{false};
    void cancel() { cancelled.store(true); }
    bool is_cancelled() const { return cancelled.load(); }
};

export class i_client {
public:
    // 同步版本（保持）
    virtual generation_result generate(...) = 0;
    
    // 真异步：返回 future + 支持取消
    virtual std::future<generation_result> generate_async(
        const nlohmann::json& config,
        const context_shared_ptr& ctx,
        const stream_callback& callback = {},
        cancel_token* token = nullptr) = 0;
};

// openai_client.cpp
std::future<generation_result> openai_client::generate_async(
    const nlohmann::json& config, const context_shared_ptr& ctx,
    const stream_callback& callback, cancel_token* token) {
    
    return std::async(std::launch::async, [this, config, ctx, callback, token]() {
        // 简单实现：定期 poll cancel_token
        // 复杂实现需要 liboai 支持中断（当前不支持）
        return generate(config, ctx, callback);
    });
}
```

**注**：真正的异步 IO 需要 liboai 底层支持（cpp-httplib 支持 async），当前只能做到"线程池 + 可取消"。

---

## 问题 7: attachment 无 base64 编码

**修复**：添加编码工具

```cpp
// message.ixx 新增
export struct attachment {
    // ... 现有字段 ...
    
    // 新增：编码为 base64
    std::string to_base64() const;
    
    // 新增：转为 OpenAI API 格式
    nlohmann::json to_openai_format() const;
};

// message.cpp
std::string attachment::to_base64() const {
    static const char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string encoded;
    size_t i = 0;
    while (i < data.size()) {
        uint32_t octet_a = static_cast<uint8_t>(data[i++]);
        uint32_t octet_b = (i < data.size()) ? static_cast<uint8_t>(data[i++]) : 0;
        uint32_t octet_c = (i < data.size()) ? static_cast<uint8_t>(data[i++]) : 0;
        
        uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;
        
        encoded += base64_chars[(triple >> 18) & 0x3F];
        encoded += base64_chars[(triple >> 12) & 0x3F];
        encoded += (i > data.size() + 1) ? '=' : base64_chars[(triple >> 6) & 0x3F];
        encoded += (i > data.size()) ? '=' : base64_chars[triple & 0x3F];
    }
    return encoded;
}

nlohmann::json attachment::to_openai_format() const {
    nlohmann::json j;
    j["type"] = "file";
    j["file"]["filename"] = name;
    j["file"]["file_data"] = to_base64();
    return j;
}
```

---

## 问题 8: standard_model getter/setter 手写易错

**修复**：字段名集中定义 + 用 json 直接访问

```cpp
// standard_model.cpp — 集中定义字段名
namespace config_keys {
    constexpr std::string_view model = "model";
    constexpr std::string_view display_name = "display_name";
    constexpr std::string_view provider = "provider";
    constexpr std::string_view base_url = "base_url";
    constexpr std::string_view api_key = "api_key";
}

// 或者：直接暴露 config 访问，减少 boilerplate
nlohmann::json standard_model::get_config() const {
    return impl->config;
}

void standard_model::set_config(const nlohmann::json& config) {
    impl->config = config;
}

// 调用方直接操作 json：
model->get_config()["temperature"] = 0.7;
// 或封装一个通用方法：
model->set_config_field("temperature", 0.7);
```

---

## 问题 9: function_call 参数过时

**修复**：支持新版 tools 接口

```cpp
// i_client.ixx
export struct tool_call {
    std::string id;
    std::string type;           // "function"
    std::string function_name;
    nlohmann::json arguments;
};

export struct generation_result {
    message_shared_ptr message;
    std::vector<tool_call> tool_calls;  // 新增
    std::optional<usage_info> usage;
    std::string finish_reason;
    std::string model;
    std::string id;
};

// openai_client.cpp — 解析 tool_calls
if (choice.contains("tool_calls")) {
    for (const auto& tc : choice["tool_calls"]) {
        tool_call call;
        call.id = tc["id"].get<std::string>();
        call.type = tc["type"].get<std::string>();
        call.function_name = tc["function"]["name"].get<std::string>();
        call.arguments = nlohmann::json::parse(tc["function"]["arguments"].get<std::string>());
        result.tool_calls.push_back(call);
    }
}
```

---

## 问题 10: catch2 体积过大

**修复方案 A**：换轻量测试框架（ doctest / ut）
**修复方案 B**：用 FetchContent 替代 submodule
**当前暂不修**：不影响功能，编译慢是已知问题（CI 可接受）

---

## 问题 11: main.cpp 硬编码 API key + 无业务逻辑

**修复**：引入配置管理（第 1 层）后解决

```cpp
// 临时方案：从环境变量读取
std::string get_api_key() {
    const char* key = std::getenv("CPPGENT_API_KEY");
    if (!key) {
        throw std::runtime_error("CPPGENT_API_KEY 环境变量未设置");
    }
    return key;
}
```

---

## 修复优先级路线

**Phase 1（必须先修）**：
1. 引入 `generation_result` + `usage_info`（修问题 1）
2. 去掉 i_model 的 client 相关（修问题 2）
3. 加 `config_validator`（修问题 3）

**Phase 2（重要但可延后）**：
4. context 加 `messages_ref()` + `find_last()`（修问题 4）
5. model_registry 加锁（修问题 5）
6. generate_async 加 cancel_token（修问题 6）

**Phase 3（锦上添花）**：
7. attachment base64（修问题 7）
8. config 字段集中定义（修问题 8）
9. tools 接口支持（修问题 9）

---

## 下一步

建议按 Phase 1 开始，逐个文件改。大哥说"开始"我就开始动手改代码。

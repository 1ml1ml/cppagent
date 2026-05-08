module;

#include <memory>
#include <string>
#include <vector>
#include <optional>

#include "nlohmann/json.hpp"

export module generation_result;

import message;

export struct usage_info {
    int prompt_tokens = 0;
    int completion_tokens = 0;
    int total_tokens = 0;
};

export struct tool_call {
    std::string id{};
    std::string type{};           // "function"
    std::string function_name{};
    nlohmann::json arguments{};
};

export struct generation_result {
    message_shared_ptr message{};                    // assistant 回复
    std::vector<tool_call> tool_calls{};             // 工具调用
    std::optional<usage_info> usage{};               // token 消耗
    std::optional<std::string> finish_reason{};      // stop/length/tool_calls
    std::string model{};                             // 实际使用的模型
    std::string id{};                                // 响应 ID
};

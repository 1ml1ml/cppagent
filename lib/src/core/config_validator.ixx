module;

#include <string>
#include <vector>
#include <stdexcept>

#include "nlohmann/json.hpp"

export module config_validator;

export class config_validation_error : public std::runtime_error {
public:
    explicit config_validation_error(const std::string& msg) : std::runtime_error(msg) {}
};

export void validate_provider_config(const nlohmann::json& config) {
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

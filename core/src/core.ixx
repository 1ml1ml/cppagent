module;

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

export module core;

export struct usage_info
{
  int total_tokens{};
  int prompt_tokens{};
  int completion_tokens{};
};

export struct tool_call
{
  std::string id{};
  std::string tool_name{};
  nlohmann::json arguments{};
};

export struct tool_call_result
{
  std::string tool_call_id{};
  bool error{};
  nlohmann::json result{};
};

export struct skill_info
{
  std::string name{};
  std::string description{};
  std::string file_path{};
};

class tool;
export using tool_shared_ptr = std::shared_ptr<tool>;

export class tool : public std::enable_shared_from_this<tool>
{
public:
	struct result
	{
		bool is_error{};
		nlohmann::json content{};
	};

public:
  virtual ~tool() = default;

public:
  virtual result call(const nlohmann::json& arguments, const std::chrono::milliseconds& timeout = std::chrono::milliseconds{ 5000 }) const = 0;

public:
  std::string name{};
  std::string description{};
  nlohmann::json input_schema{};
};

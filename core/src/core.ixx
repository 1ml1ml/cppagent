module;

#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

export module core;

// ------------------------------------------------------------------
// Forward declarations (for alias definitions)
// ------------------------------------------------------------------

class message;
class user_message;
class assistant_message;
class message_visitor;
class context;
class model_response;
class llm_api;
class llm_api_factory;
class tool;
class mcp_client;
class mcp_manager;
class skill_manager;

// ------------------------------------------------------------------
// Shared pointer / unique pointer aliases
// ------------------------------------------------------------------

export using message_shared_ptr = std::shared_ptr<message>;
export using visitor_shared_ptr = std::shared_ptr<message_visitor>;
export using context_shared_ptr = std::shared_ptr<context>;
export using model_response_shared_ptr = std::shared_ptr<model_response>;
export using api_unique_ptr = std::unique_ptr<llm_api>;
export using api_factory_shared_ptr = std::shared_ptr<llm_api_factory>;
export using tool_shared_ptr = std::shared_ptr<tool>;
export using mcp_client_shared_ptr = std::shared_ptr<mcp_client>;
export using mcp_manager_shared_ptr = std::shared_ptr<mcp_manager>;
export using skill_manager_shared_ptr = std::shared_ptr<skill_manager>;

// ------------------------------------------------------------------
// Pure data structures
// ------------------------------------------------------------------

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

// ------------------------------------------------------------------
// model_response (pure data class)
// ------------------------------------------------------------------

export class model_response
{
public:
  std::string id{};
  std::string model{};
  std::string finish_reason{};

  usage_info usage{};
  message_shared_ptr message{};
};

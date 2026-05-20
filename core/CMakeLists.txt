add_library(cppagent_core STATIC)

target_include_directories(cppagent_core PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/src/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/mimetypes/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/llm_api/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/llm_api/chat_completion_api/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/message/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/context/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/agent/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/app_config/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/skill_manager/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/mcp/jsonrpc/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/mcp/transport/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/mcp/transport/stdio_transport/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/mcp/mcp_client/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/mcp/mcp_manager/"
)

target_sources(cppagent_core
	PUBLIC
		FILE_SET cxx_modules TYPE CXX_MODULES FILES
		"src/core.ixx"
		"src/llm_api/llm_api.ixx"
		"src/llm_api/api_registry.ixx"
		"src/llm_api/chat_completion_api/chat_completion_api.ixx"
		"src/message/message.ixx"
		"src/context/context.ixx"
		"src/agent/agent.ixx"
		"src/app_config/app_config.ixx"
		"src/skill_manager/skill_manager.ixx"
		"src/mcp/jsonrpc/jsonrpc.ixx"
		"src/mcp/transport/transport.ixx"
		"src/mcp/transport/stdio_transport/stdio_transport.ixx"
		"src/mcp/mcp_client/mcp_client.ixx"
		"src/mcp/mcp_manager/mcp_manager.ixx"

	PRIVATE

		"src/mimetypes/mimetypes.hpp"
		"src/llm_api/api_registry.cpp"
		"src/llm_api/chat_completion_api/chat_completion_api.cpp"
		"src/context/context.cpp"
		"src/message/message.cpp"
		"src/agent/agent.cpp"
		"src/app_config/app_config.cpp"
		"src/skill_manager/skill_manager.cpp"
		"src/mcp/jsonrpc/jsonrpc.cpp"
		"src/mcp/transport/stdio_transport/stdio_transport.cpp"
		"src/mcp/mcp_client/mcp_client.cpp"
		"src/mcp/mcp_manager/mcp_manager.cpp"
)

target_link_libraries(cppagent_core
	PUBLIC
		nlohmann_json::nlohmann_json
		ai::sdk
	PRIVATE
		$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:pthread>
		$<$<CXX_COMPILER_ID:MSVC>:crypt32 ws2_32>
)

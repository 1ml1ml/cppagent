add_library(cppagent_core STATIC)

target_include_directories(cppagent_core PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/src/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/mimetypes/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/provider/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/provider/openai_provider/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/message/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/context/"
)

target_sources(cppagent_core
	PUBLIC
		FILE_SET cxx_modules TYPE CXX_MODULES FILES
		"src/provider/llm_provider.ixx"
		"src/provider/provider_registry.ixx"
		"src/provider/openai_provider/openai_provider.ixx"
		"src/message/message.ixx"
		"src/context/context.ixx"
	PRIVATE
		"src/mimetypes/mimetypes.hpp"
		"src/provider/llm_provider.cpp"
		"src/provider/provider_registry.cpp"
		"src/provider/openai_provider/openai_provider.cpp"
		"src/context/context.cpp"
		"src/message/message.cpp"
)

target_link_libraries(cppagent_core
	PUBLIC
		nlohmann_json::nlohmann_json
		mcp_client
		ai::sdk
	PRIVATE
		$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:pthread>
		$<$<CXX_COMPILER_ID:MSVC>:crypt32 ws2_32>
)

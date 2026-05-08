add_library(cppagent_lib STATIC)

target_include_directories(cppagent_lib PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/src/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/provider/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/provider/openai_provider/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/message/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/context/"
)

target_sources(cppagent_lib
	PUBLIC
		FILE_SET cxx_modules TYPE CXX_MODULES FILES
		"src/provider/llm_provider.ixx"
		"src/provider/provider_registry.ixx"
		"src/provider/openai_provider/openai_provider.ixx"
		"src/message/message.ixx"
		"src/context/context.ixx"
)

target_sources(cppagent_lib
	PRIVATE
		"src/mimetypes.hpp"
		"src/provider/provider_registry.cpp"
		"src/provider/openai_provider/openai_provider.cpp"
		"src/context/context.cpp"
		"src/message/message.cpp"
)

target_link_libraries(cppagent_lib
	PUBLIC
		nlohmann_json::nlohmann_json
		oai::oai
	PRIVATE
		$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:pthread>
		$<$<CXX_COMPILER_ID:MSVC>:crypt32 ws2_32>
)

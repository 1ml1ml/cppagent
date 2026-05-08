add_library(cppagent_lib STATIC)

target_include_directories(cppagent_lib PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/src/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/models/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/models/standard_model/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/message/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/context/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/clients/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/clients/openai_client/"
)

target_sources(cppagent_lib
	PUBLIC
		FILE_SET cxx_modules TYPE CXX_MODULES FILES
		"src/clients/i_client.ixx"
		"src/models/i_model.ixx"
		"src/models/model_registry.ixx"
		"src/models/standard_model/standard_model.ixx"
		"src/clients/openai_client/openai_client.ixx"
		"src/message/message.ixx"
		"src/context/context.ixx"
)

target_sources(cppagent_lib
	PRIVATE
		"src/mimetypes.hpp"
		"src/models/model_registry.cpp"
		"src/models/standard_model/standard_model.cpp"
		"src/clients/openai_client/openai_client.cpp"
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

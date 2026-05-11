add_library(mcp_client STATIC)

target_include_directories(mcp_client PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/src/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/jsonrpc/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/transport/"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/transport/stdio_transport/"
)

target_sources(mcp_client
	PUBLIC
		FILE_SET cxx_modules TYPE CXX_MODULES FILES
		"src/jsonrpc/jsonrpc.ixx"
		"src/transport/transport.ixx"
		"src/transport/stdio_transport/stdio_transport.ixx"
		"src/mcp_client/mcp_client.ixx"
		"src/mcp_manager/mcp_manager.ixx"
	PRIVATE
		"src/jsonrpc/jsonrpc.cpp"
		"src/transport/stdio_transport/stdio_transport.cpp"
		"src/mcp_client/mcp_client.cpp"
		"src/mcp_manager/mcp_manager.cpp"
)

target_link_libraries(mcp_client
	PUBLIC
		nlohmann_json::nlohmann_json
)

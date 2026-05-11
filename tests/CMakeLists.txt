add_executable(tests)

target_sources(tests
	PRIVATE
		"src/test_message.cpp"
		"src/test_context.cpp"
		"src/test_provider_registry.cpp"
		"src/test_openai_provider.cpp"
		"src/test_mcp_client.cpp"
		"src/test_mcp_manager.cpp"
)

target_link_libraries(tests
	PRIVATE
		cppagent_core
		Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(tests)

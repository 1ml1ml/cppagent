add_executable(cppagent_tests)

target_sources(cppagent_tests
	PRIVATE
		test_message.cpp
		test_context.cpp
		test_provider_registry.cpp
		test_openai_provider.cpp
)

target_link_libraries(cppagent_tests
	PRIVATE
		cppagent_lib
		Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(cppagent_tests)

add_executable(test_cppagent)

target_sources(test_cppagent
	PRIVATE
		test_message.cpp
		test_context.cpp
		test_api_registry.cpp
		test_agent.cpp
		test_app_config.cpp
)

target_link_libraries(test_cppagent
	PRIVATE
		cppagent_core
		Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(test_cppagent)

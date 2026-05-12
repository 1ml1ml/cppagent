add_executable(tests)

target_sources(tests
	PRIVATE
)

target_link_libraries(tests
	PRIVATE
		cppagent_core
		Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(tests)

add_executable(cppagent_tests)

target_sources(cppagent_tests
	PRIVATE
		test_message.cpp
		test_context.cpp
		test_model_registry.cpp
		test_standard_model.cpp
		test_openai_client.cpp
)

target_link_libraries(cppagent_tests
	PRIVATE
		cppagent_lib
		Catch2::Catch2WithMain
)

# MSVC + C++20 Modules: 测试目标需要模块扫描才能 import 我们的模块
# set_property(TARGET cppagent_tests PROPERTY CXX_SCAN_FOR_MODULES OFF)

include(Catch)
catch_discover_tests(cppagent_tests)

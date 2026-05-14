add_executable(cppagent main.cpp app_config.cpp)

target_include_directories(cppagent PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(cppagent PRIVATE cppagent_core)

cmake_minimum_required(VERSION 3.28)

project(cppagent VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

if(MSVC)
	add_compile_options(/W4 /WX- /EHsc /DNOMINMAX /utf-8)
endif()

find_package(OpenSSL REQUIRED)

# ── core ──
add_subdirectory(core)

# ── mcp ──
add_subdirectory(mcp_client)

# ── app ──
add_subdirectory(app)

# ── external ──
add_subdirectory(external/ai-sdk-cpp EXCLUDE_FROM_ALL)
add_subdirectory(external/catch2 EXCLUDE_FROM_ALL)

# ── tests ──
enable_testing()
add_subdirectory(tests EXCLUDE_FROM_ALL)

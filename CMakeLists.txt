cmake_minimum_required(VERSION 3.28)

project(cppagent VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

if(MSVC)
	add_compile_options(/W4 /WX- /EHsc /DNOMINMAX /utf-8)
endif()

find_package(nlohmann_json CONFIG REQUIRED)
find_package(oai CONFIG REQUIRED)

# ── lib ──
add_subdirectory(lib)

# ── app ──
add_subdirectory(app)

# ── tests ──
# add_subdirectory(tests)

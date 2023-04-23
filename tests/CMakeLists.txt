cmake_minimum_required(VERSION 3.9.2)

project(safe_tests LANGUAGES CXX)

# Detect if used in add_subdirectory() or install space
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
	find_package(safe CONFIG REQUIRED)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Warnings)

include(add_package)
add_package(
	doctest
	doctest
	https://github.com/onqtam/doctest
	v2.4.9
	scripts/cmake
)

add_executable(safe_tests test_main.cpp test_readme.cpp test_safe.cpp)
target_link_libraries(safe_tests PRIVATE safe::safe doctest::doctest)
target_set_warnings(safe_tests ENABLE ALL AS_ERROR ALL DISABLE Annoying)
target_compile_features(safe_tests INTERFACE cxx_std_17)

include(CTest)
include(doctest)
doctest_discover_tests(safe_tests)

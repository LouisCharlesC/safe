cmake_minimum_required(VERSION 3.9.2)

project(safe_tests LANGUAGES CXX)

# Detect if used in add_subdirectory() or install space
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
	find_package(safe CONFIG REQUIRED)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(Warnings)

include(FetchContent)
FetchContent_Declare(
        DocTest
        GIT_REPOSITORY "https://github.com/onqtam/doctest"
        GIT_TAG "v2.4.9"
)
FetchContent_MakeAvailable(DocTest)

add_executable(safe_tests test_main.cpp test_readme.cpp test_safe.cpp)
target_link_libraries(safe_tests PRIVATE safe::safe doctest)
target_set_warnings(safe_tests ENABLE ALL AS_ERROR ALL DISABLE Annoying)
target_compile_features(safe_tests INTERFACE cxx_std_17)

include(CTest)
add_test(NAME safe.safe_tests COMMAND safe_tests -s)
include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(safe_tests)

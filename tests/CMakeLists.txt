cmake_minimum_required(VERSION 3.9.2)

project(safe_tests LANGUAGES CXX)

include(CTest)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(Warnings)

find_package(safe 1.0 REQUIRED)
find_package(doctest REQUIRED)

set(TEST_MAIN unit)

add_executable(${TEST_MAIN} test_main.cpp test_readme.cpp test_safe.cpp)
target_link_libraries(${TEST_MAIN} PRIVATE safe::safe doctest::doctest)
target_set_warnings(${TEST_MAIN} ENABLE ALL AS_ERROR ALL DISABLE Annoying)
set_target_properties(
	${TEST_MAIN}
	PROPERTIES
		CXX_STANDARD 17
		CXX_EXTENSIONS NO
)

add_test(NAME ${PROJECT_NAME}.${TEST_MAIN} COMMAND ${TEST_MAIN})

# Adds a 'coverage' target to compile ${TEST_MAIN}.
include(CoverageTarget)
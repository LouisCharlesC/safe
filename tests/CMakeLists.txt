cmake_minimum_required(VERSION 3.9.2)

project(tests LANGUAGES CXX)

include(CTest)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(Warnings)

find_package(safe 1.0 CONFIG REQUIRED)
find_package(doctest CONFIG REQUIRED )

add_executable(${PROJECT_NAME} test_main.cpp test_readme.cpp test_safe.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE safe::safe doctest::doctest)
target_set_warnings(${PROJECT_NAME} ENABLE ALL AS_ERROR ALL DISABLE Annoying)
set_target_properties(
	${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD 17
		CXX_EXTENSIONS NO
)

add_test(NAME safe.${PROJECT_NAME} COMMAND ${PROJECT_NAME} -s)

set(TEST_MAIN ${PROJECT_NAME})
# Adds a 'coverage' target to compile ${TEST_MAIN}.
include(CoverageTarget)
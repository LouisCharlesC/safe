cmake_minimum_required(VERSION 3.9.2)

project(safe_tests LANGUAGES CXX)

include(CTest)
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
target_link_libraries(safe_tests PRIVATE safe doctest)
target_set_warnings(safe_tests ENABLE ALL AS_ERROR ALL DISABLE Annoying)
set_target_properties(
	safe_tests
	PROPERTIES
		CXX_STANDARD 17
		CXX_EXTENSIONS NO
)

add_test(NAME safe.safe_tests COMMAND safe_tests -s)

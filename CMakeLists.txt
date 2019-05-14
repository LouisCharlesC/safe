cmake_minimum_required (VERSION 2.8.8)

if (CMAKE_VERSION VERSION_LESS "3.1")
  add_definitions(-std=c++11)
else()
  set(CMAKE_CXX_STANDARD 11)
endif()

project (safe)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 2)
set(VERSION_PATCH 0)

find_package(Doxygen QUIET)
if (DOXYGEN_FOUND)
	set(DOXYFILE_TO_CONFIGURE ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
	set(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

	configure_file(${DOXYFILE_TO_CONFIGURE} ${DOXYFILE} @ONLY)
	
	add_custom_target(doc COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE})
endif()

include_directories(include)

find_package(GTest QUIET)
if(GTEST_FOUND)
	message(STATUS "Building Tests")
	find_package(Threads REQUIRED)
	add_executable(tests tests/tests.cpp tests/test_readme.cpp tests/test_lockable.cpp tests/test_safe.cpp)
	target_link_libraries(tests gmock gtest ${CMAKE_THREAD_LIBS_INIT})
endif()

install(DIRECTORY include/safe DESTINATION include)

cmake_minimum_required (VERSION 2.8.8)

if (CMAKE_VERSION VERSION_LESS "3.1")
  add_definitions(-std=c++11)
else()
  set(CMAKE_CXX_STANDARD 11)
endif()

project (safe)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 1)

find_package(Doxygen)
if (${DOXYGEN_FOUND})
	set(DOXYFILE_TO_CONFIGURE ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
	set(DOXYFILE ${CMAKE_BINARY_DIR}/Doxyfile)

	configure_file(${DOXYFILE_TO_CONFIGURE} ${DOXYFILE} @ONLY)
	
	add_custom_target(doc COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE})
endif(${DOXYGEN_FOUND})

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

include_directories(include)

add_executable(tests tests/tests.cpp tests/test_safe.cpp)
target_link_libraries(tests gmock gtest ${CMAKE_THREAD_LIBS_INIT})

install(DIRECTORY include/safe DESTINATION include)
cmake_minimum_required(VERSION 3.10)

project(safe VERSION 0.2.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


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
    enable_testing()
    add_subdirectory("tests")
endif()

install(DIRECTORY include/safe DESTINATION include)

cmake_minimum_required(VERSION 3.9.2)

cmake_policy(SET CMP0048 NEW)
project(safe VERSION 1.1.0 LANGUAGES CXX)
option(BUILD_TESTING "Build tests" ON)

include(GNUInstallDirs)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE
	$<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_11)
add_library(safe::safe ALIAS safe)

if(BUILD_TESTING)
	enable_testing()
	add_subdirectory(tests)
endif()

include(InstallTarget)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
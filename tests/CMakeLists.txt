message(STATUS "Building Tests")

find_package(Threads REQUIRED)

add_executable(tests test_readme.cpp test_lockable.cpp test_safe.cpp)
target_link_libraries(tests PRIVATE gmock "${GTEST_BOTH_LIBRARIES}" Threads::Threads)
add_test(NAME Tests COMMAND tests)


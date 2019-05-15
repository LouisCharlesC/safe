message(STATUS "Building Tests")

add_executable(tests tests.cpp test_readme.cpp test_lockable.cpp test_safe.cpp)
target_link_libraries(tests PRIVATE gmock GTest::GTest)
add_test(NAME Tests COMMAND tests)


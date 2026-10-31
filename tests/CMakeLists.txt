file(GLOB TEST_SOURCES "test_*.cpp")

add_executable(my_lib_tests ${TEST_SOURCES})

target_link_libraries(my_lib_tests PRIVATE my_lib gtest_main)

include(GoogleTest)
gtest_discover_tests(my_lib_tests)

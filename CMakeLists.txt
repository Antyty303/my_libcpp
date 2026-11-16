cmake_minimum_required(VERSION 3.20)
project(my_libcpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(my_libcpp INTERFACE)
target_include_directories(my_libcpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(CTest)
if(BUILD_TESTING)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory(3rdparty/gtest)
    
    add_subdirectory(tests)
endif()
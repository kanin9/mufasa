cmake_minimum_required(VERSION 3.10)
project(mufasa)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -static-libstdc++")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Use 'cmake . -DCMAKE_BUILD_TYPE=Release' for maximum performance")

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
		cmake_policy(SET CMP0135 NEW)
endif()

option(BUILD_TESTS "Build tests for the engine" OFF)

set(SRC_DIR src)
set(TESTS_DIR tests)
file(GLOB ALL_FILES ${SRC_DIR}/*.cpp ${SRC_DIR}/*.hpp)
file(GLOB ALL_FILES_EXCEPT_MAIN ${SRC_DIR}/*.cpp {SRC_DIR}/*.hpp)
list(FILTER ALL_FILES_EXCEPT_MAIN EXCLUDE REGEX "main.cpp")

add_executable(${PROJECT_NAME} ${ALL_FILES})

if(BUILD_TESTS)
   include(FetchContent)
   FetchContent_Declare(
     googletest
     URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
   )
   # For Windows: Prevent overriding the parent project's compiler/linker settings
   set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
   FetchContent_MakeAvailable(googletest)

   enable_testing()
   add_executable(tests ${TESTS_DIR}/tests.cpp ${ALL_FILES_EXCEPT_MAIN})

   target_link_libraries(tests GTest::gtest_main)
   include(GoogleTest)
   gtest_add_tests(TARGET tests SOURCES ${TESTS_DIR}/tests.cpp)
endif(BUILD_TESTS)

cmake_minimum_required(VERSION 3.20)

project(cpp_calculator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CALCULATOR_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

function(calculator_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
        if(CALCULATOR_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        if(CALCULATOR_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

add_library(calculator_core STATIC calculator/calculator.cpp)
target_include_directories(calculator_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/calculator)
calculator_set_warnings(calculator_core)

add_executable(calculator calculator/main.cpp)
target_link_libraries(calculator PRIVATE calculator_core)
calculator_set_warnings(calculator)

include(CTest)

if(BUILD_TESTING)
    add_executable(calculator_tests tests/calculator_tests.cpp tests/test_main.cpp)
    target_link_libraries(calculator_tests PRIVATE calculator_core)
    target_include_directories(calculator_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    calculator_set_warnings(calculator_tests)

    add_test(NAME calculator_tests COMMAND calculator_tests)
endif()

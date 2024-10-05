cmake_minimum_required(VERSION 3.10)

project(MonadicOperations LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(monadic_operations)

enable_testing()

add_subdirectory(tests)


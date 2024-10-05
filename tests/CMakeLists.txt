include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(tests tests.cpp)

target_link_libraries(tests gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)

cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(hello)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(nlohmann_json_url "https://github.com/nlohmann/json.git")
set(nlohmann_json_version "v3.11.3")  # Replace with your desired version


include(FetchContent)

## Automatically download the dependencies from github
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2  # Specify the version you want to use
)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

FetchContent_MakeAvailable(json googletest)

# Default targe
add_executable(hello src/hello_world.cpp)

# Target for running echo exercise for maelstorm
add_executable(echo src/maelstorm/echo.cpp)
target_link_libraries(echo PRIVATE nlohmann_json::nlohmann_json)

set_target_properties(echo PROPERTIES
  LINK_SEARCH_START_STATIC ON
  LINK_SEARCH_END_STATIC ON
)

#set(CMAKE_EXE_LINKER_FLAGS "-static")

# Enable testing
enable_testing()

# Test executable
add_executable(echo_test test/maelstorm/test_rpc.cpp)  # Replace test_main.cpp with your test files
target_include_directories(echo_test PUBLIC src/maelstorm)

target_link_libraries(echo_test PRIVATE
  nlohmann_json::nlohmann_json
  GTest::gtest_main
)

# Discover tests
include(GoogleTest)
gtest_discover_tests(echo_test)

# Custom target to run tests
add_custom_target(run_tests
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  DEPENDS echo_test
)

# Run tests as part of the build process
add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} --build . --target run_tests
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Running tests..."
)

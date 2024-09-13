cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(hello)
set(CMAKE_CXX_STANDARD 20)
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
add_executable(maps src/maps.cpp)
add_executable(move_1 src/move_example_1.cpp)
add_executable(move_2 src/move_example_2.cpp)
add_executable(move_3 src/memory_allocation_vs_pooling.cpp)
add_executable(move_4 src/pointer_indirection.cpp)

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
# In future, can create more targets to divide up tests into multiple suits
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

# Find clang-format
find_program(CLANG_FORMAT "clang-format")
if(NOT CLANG_FORMAT)
  message(WARNING "clang-format not found, auto-formatting will not be available")
endif()

if(CLANG_FORMAT)
  # Get all project source files
  file(GLOB_RECURSE ALL_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/*.h
    ${PROJECT_SOURCE_DIR}/src/*.hpp
    ${PROJECT_SOURCE_DIR}/test/*.cpp
    ${PROJECT_SOURCE_DIR}/test/*.h
    ${PROJECT_SOURCE_DIR}/test/*.hpp
  )

  add_custom_target(
    format
    COMMAND ${CLANG_FORMAT} -i -style=file ${ALL_SOURCE_FILES}
    COMMENT "Auto-formatting codebase..."
  )

 # add_custom_target(
 #   format
 #   COMMENT "Auto-formatting codebase..."
 # )

 # # Add a custom command for each source file
 # foreach(SOURCE_FILE ${ALL_SOURCE_FILES})
 #   add_custom_command(
 #     TARGET format
 #     COMMAND ${CLANG_FORMAT} -i -style=file ${SOURCE_FILE}
 #     COMMENT "Formatting ${SOURCE_FILE}"
 #   )
 # endforeach()

  # Run auto-formatting before building the main target
  add_dependencies(${PROJECT_NAME} format)
endif()

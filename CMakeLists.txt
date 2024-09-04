cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(hello LANGUAGES CXX)
set(nlohmann_json_url "https://github.com/nlohmann/json.git")
set(nlohmann_json_version "v3.11.3")  # Replace with your desired version


include(FetchContent)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2  # Specify the version you want to use
)

FetchContent_MakeAvailable(json)

add_executable(hello src/hello_world.cpp)

add_executable(echo src/maelstorm/echo.cpp)
target_link_libraries(echo PRIVATE nlohmann_json::nlohmann_json)

set_target_properties(echo PROPERTIES
  LINK_SEARCH_START_STATIC ON
  LINK_SEARCH_END_STATIC ON
)

#set(CMAKE_EXE_LINKER_FLAGS "-static")

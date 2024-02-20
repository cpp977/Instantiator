cmake_minimum_required(VERSION 3.20)

project(Calculator LANGUAGES CXX)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Calculator.cpp")

add_executable(${PROJECT_NAME} ${sources})

add_subdirectory("../Math" "./MathBuild")

target_link_libraries(${PROJECT_NAME} Math)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.hpp)
message(INFO ": srcs=${ALL_SOURCE_FILE}")
add_custom_target(
        clangformat
        COMMAND /usr/bin/clang-format
        -style=file
        -i
        ${ALL_SOURCE_FILES}
)

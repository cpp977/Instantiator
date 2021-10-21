cmake_minimum_required(VERSION 3.13.4)
project(Instantiator VERSION 0.1.0 LANGUAGES CXX)

include(cmake/CPM.cmake)
CPMAddPackage(
  NAME termcolor
  VERSION 2.0.0
  GIT_REPOSITORY https://github.com/ikalnytskyi/termcolor
  )

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

find_package(Clang REQUIRED)
add_executable(Instantiator Instantiator.cpp
  src/Injection.cpp
  src/Callbacks/InjectInstantiation.cpp
  src/Callbacks/GetNeededInstantiations.cpp)

message(STATUS ${CMAKE_SOURCE_DIR})
target_include_directories(Instantiator PRIVATE ${LLVM_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include)
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
target_compile_definitions(Instantiator PRIVATE ${LLVM_DEFINITIONS_LIST})

target_link_libraries(Instantiator PRIVATE
  LLVM
  clang-cpp
  termcolor::termcolor
  )


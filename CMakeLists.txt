cmake_minimum_required(VERSION 3.13.4)
project(Instantiator VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)

include(cmake/CPM.cmake)
CPMAddPackage(
  NAME termcolor
  GIT_TAG master
  # GIT_REPOSITORY https://github.com/ikalnytskyi/termcolor
  GIT_REPOSITORY https://github.com/cpp977/termcolor
  )

CPMAddPackage(
  NAME indicator
  VERSION 2.2
  GIT_REPOSITORY https://github.com/p-ranav/indicators
  )

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Major: ${LLVM_VERSION_MAJOR}")
message(STATUS "Minor: ${LLVM_VERSION_MINOR}")
message(STATUS "Patch: ${LLVM_VERSION_PATCH}")
message(STATUS "Suffix: ${LLVM_VERSION_SUFFIX}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Using llvm headers in: ${LLVM_INCLUDE_DIRS}")
message(STATUS "Using llvm libs in: ${LLVM_LIB_DIRS}")

find_package(Clang REQUIRED)

message(STATUS "Using clang header in: ${CLANG_INCLUDE_DIRS}")

add_executable(Instantiator Instantiator.cpp
  src/Injection.cpp
  src/Param.cpp
  src/Callbacks/InjectInstantiation.cpp
  src/Callbacks/GetNeededInstantiations.cpp
  src/Callbacks/DeleteInstantiations.cpp
  src/Matcher/Matcher.cpp
  src/Actions/AllASTBuilderAction.cpp
  src/Actions/ASTBuilderAction.cpp
  src/Actions/DependencyAction.cpp
  src/ASTCreation.cpp
  src/Parsing.cpp
  src/Template.cpp)

# set_target_properties(Instantiator PROPERTIES SUFFIX ${LLVM_VERSION_MAJOR})

message(STATUS ${CMAKE_SOURCE_DIR})
target_include_directories(Instantiator PRIVATE ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include)
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})

target_compile_definitions(Instantiator PRIVATE ${LLVM_DEFINITIONS_LIST})
target_compile_definitions(Instantiator PRIVATE INSTANTIATOR_LLVM_MAJOR=${LLVM_VERSION_MAJOR})
target_compile_definitions(Instantiator PRIVATE INSTANTIATOR_LLVM_MINOR=${LLVM_VERSION_MINOR})
target_compile_definitions(Instantiator PRIVATE INSTANTIATOR_LLVM_PATCH=${LLVM_VERSION_PATCH})
target_compile_definitions(Instantiator PRIVATE INSTANTIATOR_LLVM_SUFFIX=${LLVM_VERSION_SUFFIX})
target_compile_features(Instantiator PRIVATE cxx_std_17)
target_compile_options(Instantiator PRIVATE -fno-rtti)

llvm_map_components_to_libnames(llvm_libs support core frontendopenmp)

target_link_libraries(Instantiator PRIVATE
  clang-cpp
  termcolor::termcolor
  indicators::indicators
  )

if (${CLANG_LINK_CLANG_DYLIB})
  message(STATUS "Linking against dynamic llvm.")
  target_link_libraries(Instantiator PRIVATE LLVM)
else()
    message(STATUS "Linking against static llvm components.")
  target_link_libraries(Instantiator PRIVATE ${llvm_libs})
endif()

include(GNUInstallDirs)
install(TARGETS Instantiator)

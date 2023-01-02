cmake_minimum_required(VERSION 3.16)

project(bclist VERSION 1.1
        DESCRIPTION "Print the bytecode list of the compiled Lua script"
        LANGUAGES CXX C)

option(BCLIST_CLI "Build a console application" OFF)

add_subdirectory(src)

if (BCLIST_CLI)
  find_package(args REQUIRED)

  add_executable(bclist-cli src/main.cpp)
  target_compile_features(bclist-cli PRIVATE cxx_std_20)
  target_link_libraries(bclist-cli PRIVATE bclist taywee::args)
endif()
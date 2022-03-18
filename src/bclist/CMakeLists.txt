cmake_minimum_required(VERSION 3.16)

project(bclist VERSION 1.1
        DESCRIPTION "Print the bytecode list of the compiled Lua script"
        LANGUAGES CXX C)

option(BCLIST_CLI "Build a console application" OFF)

find_package(fmt CONFIG REQUIRED)
find_package(dislua REQUIRED)

add_library(bclist
  src/bclist.cpp
  src/bclist/lj.cpp)

target_include_directories(bclist PUBLIC src)
target_link_libraries(bclist PUBLIC fmt::fmt dislua)
target_compile_features(bclist PUBLIC cxx_std_20)

if (MSVC)
  # warning level 4 and all warnings as errors
  target_compile_options(bclist PRIVATE
    /W4
    /WX)
else()
  # lots of warnings and all warnings as errors
  target_compile_options(bclist PRIVATE
    -Werror
    -pedantic-errors

    -Wall
    -Wextra
    -Wpedantic

    -Wcast-align
    -Wcast-qual
    -Wconversion
    -Wctor-dtor-privacy
    # -Wduplicated-branches
    # -Wduplicated-cond
    -Wextra-semi
    -Wfloat-equal
    # -Wlogical-op
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Woverloaded-virtual
    -Wredundant-decls
    -Wsign-conversion
    -Wsign-promo)
endif()

if (BCLIST_CLI)
  find_package(args REQUIRED)

  add_executable(bclist-cli src/main.cpp)
  target_compile_features(bclist-cli PRIVATE cxx_std_20)
  target_link_libraries(bclist-cli PRIVATE bclist taywee::args)
endif()
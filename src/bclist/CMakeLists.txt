cmake_minimum_required(VERSION 3.16)
include(FetchContent)

project(bclist VERSION 1.2
        DESCRIPTION "Print the bytecode list of the compiled Lua script"
        LANGUAGES CXX C)

option(BCLIST_CLI "Build a console application" OFF)

add_subdirectory(src)

if (BCLIST_CLI)
    set(ARGS_BUILD_EXAMPLE OFF)
    set(ARGS_BUILD_UNITTESTS OFF)

    FetchContent_Declare(
        args
        GIT_REPOSITORY https://github.com/Taywee/args.git
        GIT_TAG 9e1180a7231c2bc8aee3a16fbcb0948ba28d8dfa
    )
    FetchContent_MakeAvailable(args)

    add_executable(bclist-cli src/main.cpp)
    target_compile_features(bclist-cli PRIVATE cxx_std_20)
    target_link_libraries(bclist-cli PRIVATE bclist args)
endif()
cmake_minimum_required(VERSION 3.16)

project(luad VERSION 0.12
        DESCRIPTION "Disassembly Lua compiled scripts"
        LANGUAGES CXX C)

find_package(OpenGL REQUIRED)
find_package(glfw3  REQUIRED)
find_package(gl3w   REQUIRED)
find_package(fmt    REQUIRED)
find_package(dislua REQUIRED)

include(cmake/include_imgui.cmake)

set(LUAD_SOURCE
  src/main.cpp
  src/file.cpp
  src/bclist.cpp
  src/wininterface.cpp

  src/bclist/lj.cpp
  src/luajit/window.cpp
)

set(SOURCE ${SOURCE} ${LUAD_SOURCE})

set(HEADER_DIR ${HEADER_DIR}
  src
  src/luajit
)

add_executable(luad ${SOURCE})

target_compile_definitions(luad PRIVATE IMGUI_IMPL_OPENGL_LOADER_GL3W)
target_include_directories(luad PRIVATE ${HEADER_DIR})
target_link_libraries(luad PRIVATE ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS} glfw fmt::fmt dislua gl3w)
target_compile_features(luad PRIVATE cxx_std_17)
set_target_properties(luad PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output/"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/output/"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/output/"
)

if (MSVC)
  list(APPEND FLAGS /W4 /WX)
else()
  # https://habr.com/ru/post/490850/
  list(APPEND FLAGS 
    -Werror
    -pedantic-errors

    -Wall
    -Wextra
    -Wpedantic

    -Wcast-align
    -Wcast-qual
    -Wconversion
    -Wctor-dtor-privacy
    -Wduplicated-branches
    -Wduplicated-cond
    -Wextra-semi
    -Wfloat-equal
    -Wlogical-op
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Woverloaded-virtual
    -Wredundant-decls
    -Wsign-conversion
    -Wsign-promo
  )
endif()
string(REPLACE ";" " " FLAGS_STR "${FLAGS}")

set_source_files_properties(${LUAD_SOURCE} PROPERTIES COMPILE_FLAGS "${FLAGS_STR}")
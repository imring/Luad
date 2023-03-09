cmake_minimum_required(VERSION 3.16)

project(Luad
        VERSION 0.21
        DESCRIPTION "Disassembler for compiled Lua scripts"
        LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(QT_VERSION 6)
find_package(Qt${QT_VERSION} REQUIRED COMPONENTS Core Widgets)

add_subdirectory(src)
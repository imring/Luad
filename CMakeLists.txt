cmake_minimum_required(VERSION 3.16)

project(Luad
        VERSION 0.22
        DESCRIPTION "Disassembler for compiled Lua scripts"
        LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(QT_VERSION 6)
find_package(Qt${QT_VERSION} REQUIRED COMPONENTS Core Widgets)

include(submodules/qhexedit2.cmake)

add_subdirectory(src)
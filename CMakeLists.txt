cmake_minimum_required(VERSION 3.16)

project(luad LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(QT_VERSION 6)
find_package(Qt${QT_VERSION} REQUIRED COMPONENTS Widgets)

add_subdirectory(src)
cmake_minimum_required(VERSION 3.20)
project(Luad
        VERSION 0.20
        DESCRIPTION "Disassembler for compiled Lua scripts"
        LANGUAGES CXX C)

find_package(OpenGL REQUIRED)
find_package(dislua REQUIRED)
find_package(fmt REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

# include nfd
add_subdirectory(external/nativefiledialog-extended)

# include glfwpp
set(GLFWPP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # disable building GLFWPP examples
add_subdirectory(external/glfwpp)
add_subdirectory(external/glfwpp/external/glew-cmake)
add_subdirectory(external/glfwpp/external/imgui)

# include bclist
add_subdirectory(src/bclist)

add_executable(luad
  src/main.cpp
  src/file.cpp

  src/gui/disassembly.cpp
  src/gui/settings.cpp
  src/gui/wininterface.cpp

  src/imgui_addons/imgui_addons.cpp
  src/imgui_addons/imgui_notf.cpp

  # external
  external/ImGuiColorTextEdit/TextEditor.cpp)
target_include_directories(luad PRIVATE
  external/imgui_club/imgui_memory_editor
  external/ImGuiColorTextEdit)
target_compile_features(luad PRIVATE cxx_std_20)
target_link_libraries(luad PRIVATE ${OPENGL_LIBRARIES}
    libglew_static GLFWPP Imgui nfd # window
    dislua fmt::fmt bclist          # parse/show
    nlohmann_json::nlohmann_json)   # config
target_compile_definitions(luad PRIVATE
    GLEW_STATIC
    UNICODE
    _UNICODE)

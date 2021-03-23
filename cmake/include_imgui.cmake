set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/deps/imgui)
set(IMGUI_TEXTEDIT_DIR ${CMAKE_SOURCE_DIR}/deps/ImGuiColorTextEdit)

if (NOT EXISTS ${IMGUI_DIR} OR NOT EXISTS ${IMGUI_TEXTEDIT_DIR})
    message(STATUS "Updating submodules")
    execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMAND git submodule init)
    execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMAND git submodule update)
endif()

# set(HEADER_DIR ${HEADER_DIR}
include_directories(SYSTEM
  ${IMGUI_DIR}
  ${IMGUI_DIR}/backends
  ${IMGUI_TEXTEDIT_DIR}
)

set(SOURCE ${SOURCE}
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_demo.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
  
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
  ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp

  ${IMGUI_TEXTEDIT_DIR}/TextEditor.cpp
)
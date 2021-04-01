// Luad - Disassembler for compiled Lua scripts.
// https://github.com/imring/Luad
// Copyright (C) 2021 Vitaliy Vorobets

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <filesystem>
#include <fstream>
#include <iostream>

#include "GL/gl3w.h"
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "dislua/dislua.hpp"

#include "luajit/window.hpp"
#include "wininterface.hpp"

std::unique_ptr<luad::wininterface> create_win(luad::luac_file &f) {
  if (f.error())
    return {};
  switch (f.info()->compiler()) {
  case dislua::DISLUA_LUAJIT:
    return std::make_unique<luad::lj::window>(f);
  default:
    return std::make_unique<luad::wininterface>(f);
  }
}

int main(int argc, char *argv[]) {
  glfwSetErrorCallback([](int error, const char *description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
  });

  IM_UNUSED(argc); IM_UNUSED(argv);
  std::filesystem::path p = std::filesystem::u8path("./шпора_imgui.luac");

  //if (argc != 2) {
  //  std::cout << "USAGE: luad [file]" << std::endl;
  //  return 0;
  //}
  //std::filesystem::path p = argv[1];

  luad::luac_file main_file(p);
  switch (main_file.error()) {
  case luad::luac_file::LFE_IS_FILE:
    std::cerr << "The path isn't a file or a file doesn't exist." << std::endl;
    return 1;
  case luad::luac_file::LFE_OPEN_FILE:
    std::cerr << "Error opening file." << std::endl;
    return 1;
  case luad::luac_file::LFE_PARSE:
    std::cerr << "Unknown compiler of lua script." << std::endl;
    return 1;
  default:
    break;
  }
  auto main_win = create_win(main_file);

  if (glfwInit() == GLFW_FALSE) {
    std::cerr << "GLFW initialization error." << std::endl;
    return 1;
  }

#ifdef __APPLE__
  // GL 3.2 + GLSL 150
  const char *glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#else
  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // 3.0+ only
#endif

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  if (!main_win->initialize())
    return 1;

  glfwSwapInterval(1);
  if (gl3wInit() != 0) {
    std::cerr << "Failed to initialize OpenGL loader." << std::endl;
    return 1;
  }

  ImGui_ImplOpenGL3_Init(glsl_version);

  while (!main_win->should_close()) {
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    main_win->render();

    // Rendering
    ImGui::Render();
    int display[2];
    main_win->get_framebuf_size(display);
    glViewport(0, 0, display[0], display[1]);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    main_win->swap_buffers();
  }
  main_win->destroy();

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();

  return 0;
}
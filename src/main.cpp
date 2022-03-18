// Luad - Disassembler for compiled Lua scripts.
// https://github.com/imring/Luad
// Copyright (C) 2021-2022 Vitaliy Vorobets
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "main.hpp"
#include "os/console.hpp"

#include <GL/glew.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <nfd.hpp>

#include "gui/wininterface.hpp"

int main(int argc, char *argv[]) {
  using namespace std::chrono_literals;

  hide_console();
  const auto GLFW = glfw::init();
  glfw::WindowHints{
      .contextVersionMajor = 4,
      .contextVersionMinor = 6,
      .openglProfile = glfw::OpenGlProfile::Core}.apply();

  NFD::Guard nfdGuard;

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  luad::wininterface main_win;
  if (argc == 2)
    main_win.open_file(argv[1]);

  ImGui_ImplOpenGL3_Init();
  glfw::swapInterval(1);
  if (glewInit() != GLEW_OK)
    throw std::runtime_error{"Could not initialize GLEW"};

  static ImVec4 clear_color{0.45f, 0.55f, 0.60f, 1.00f};
  while (!main_win.shouldClose()) {
    glfw::pollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    main_win.render();

    // Rendering
    ImGui::Render();
    const auto [dx, dy] = main_win.getFramebufferSize();
    glViewport(0, 0, dx, dy);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    main_win.swapBuffers();
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  return 0;
}
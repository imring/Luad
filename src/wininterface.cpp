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

#ifdef _WIN32
#include <Windows.h>
#endif

#include "main.hpp"
#include "wininterface.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "fmt/os.h"

#ifdef _WIN32
std::string from_widechar(std::wstring_view s) {
  int size_needed = WideCharToMultiByte(
      CP_UTF8, 0, s.data(), static_cast<int>(s.size()), NULL, 0, NULL, NULL);
  std::string result(size_needed, '\0');
  WideCharToMultiByte(CP_UTF8, 0, s.data(), static_cast<int>(s.size()),
                      result.data(), size_needed, NULL, NULL);
  return result;
}
#endif

bool luad::wininterface::initialize() {
#ifdef _WIN32
  std::wstring filename = path().filename().wstring();
  std::wstring wname_window = fmt::format(L"Luad {}", filename);
  std::string name_window = from_widechar(wname_window);
#else
  std::string filename = path().filename().string();
  std::string name_window = fmt::format("Luad {}", filename);
#endif
  win = glfwCreateWindow(800, 600, name_window.c_str(), nullptr, nullptr);
  if (win == nullptr) {
    std::cerr << "Failed to create GLFW window." << std::endl;
    return false;
  }

  glfwMakeContextCurrent(win);
  ImGui_ImplGlfw_InitForOpenGL(win, true);
  ImGui::StyleColorsDark();

  ImGuiIO &io = ImGui::GetIO();
  static const ImWchar ranges[] = {0x0020, 0x00FF, 0x0400, 0x052F,
                                   0x2DE0, 0x2DFF, 0xA640, 0xA69F,
                                   0x2013, 0x2122, 0};
  ImFont *font = io.Fonts->AddFontFromFileTTF(
      "./fonts/LiberationMono-Regular.ttf", 13.f, nullptr, ranges);
  if (font == nullptr) {
    std::cerr << "Failed to load the font in the path \"./fonts/LiberationMono-Regular.ttf\"." << std::endl;
    return false;
  }

  io.Fonts->Build();
  return true;
}

void luad::wininterface::render_menubar() {
  bool open_about = false;
  if (!ImGui::BeginMenuBar())
    return;

  if (ImGui::BeginMenu("File")) {
    ImGui::MenuItem("Save", "Ctrl+S", nullptr, false);
    ImGui::Separator();
    if (ImGui::MenuItem("Quit"))
      set_should_close(true);
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("Help")) {
    if (ImGui::MenuItem("About"))
      open_about = true;
    ImGui::EndMenu();
  }

  ImGui::EndMenuBar();

  if (open_about) {
    ImGui::OpenPopup("About");
    ImVec2 winsize = get_window_size();
    winsize.x /= 2.f;
    winsize.y /= 2.f;
    ImGui::SetNextWindowPos(winsize, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  }

  if (ImGui::BeginPopupModal("About", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_NoMove)) {
    ImGui::Text("Luad - Disassembler for compiled Lua scripts.");
    ImGui::Text("Version: %d.%02d (%d)", LUAD_VERSION / 100, LUAD_VERSION % 100,
                LUAD_VER_DATE);
    ImGui::Text("Luad is licensed under the GNU General Public License v3.0.");
    ImGui::NewLine();

    ImGui::Text("Authors:");
    ImGui::Text("Vitaliy Vorobets <https://github.com/imring>");

    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 100) / 2);
    if (ImGui::Button("Close", ImVec2(100, 0)))
      ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
  }
}

void luad::wininterface::render() {
  int width, height;
  glfwGetWindowSize(win, &width, &height);

  ImGui::SetNextWindowSize(get_window_size(), ImGuiCond_Always);
  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
  if (!ImGui::Begin("##luad", nullptr,
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoTitleBar |
                        ImGuiWindowFlags_MenuBar)) {
    ImGui::End();
    return;
  }

  render_menubar();

  size_t di_size = decompilers_info.size();
  size_t pi_size = plugins_info.size();

  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() / 5);
  ImGui::BeginChild("##left");
  render_left_panel();
  ImGui::EndChild();

  ImGui::NextColumn();
  if (ImGui::BeginTabBar("##menu")) {
    // decompilers info
    for (size_t i = 0; i < di_size; ++i)
      if (ImGui::BeginTabItem(decompilers_info[i].name.c_str())) {
        decompilers_info[i].func(this);
        ImGui::EndTabItem();
      }

    // plugins
    for (size_t i = 0; i < pi_size; ++i)
      if (ImGui::BeginTabItem(plugins_info[i].name.c_str())) {
        plugins_info[i].func(this);
        ImGui::EndTabItem();
      }

    ImGui::EndTabBar();
  }
  ImGui::Columns(1);

  ImGui::End();
}

ImVec2 luad::wininterface::get_window_size() {
  int width, height;
  glfwGetWindowSize(win, &width, &height);
  return ImVec2(static_cast<float>(width), static_cast<float>(height));
}
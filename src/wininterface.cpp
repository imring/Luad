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
  if (ImGui::BeginMenu("View")) {
    for (wininfo &wi : decompilers_info)
      ImGui::MenuItem(wi.name.c_str(), nullptr, &wi.view);

    if (plugins_info.size() > 0)
      ImGui::Separator();
    for (wininfo &wi: plugins_info)
      ImGui::MenuItem(wi.name.c_str(), nullptr, &wi.view);
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
    ImVec2 winsize = ImGui::GetMainViewport()->WorkSize;
    winsize.x /= 2.f;
    winsize.y /= 2.f;
    ImGui::SetNextWindowPos(winsize, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  }

  if (ImGui::BeginPopupModal("About", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_NoMove)) {
    ImGui::Text("Luad - Disassembler for compiled Lua scripts.");
    ImGui::Text("Version: %d.%02d.", LUAD_VERSION / 100, LUAD_VERSION % 100);
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
  const ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_MenuBar               | ImGuiWindowFlags_NoDocking  |
      ImGuiWindowFlags_NoTitleBar            | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoResize              | ImGuiWindowFlags_NoMove     |
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus ;

  ImGui::Begin("Luad", nullptr, window_flags);
  ImGui::PopStyleVar(3);

  ImGuiID dockid = ImGui::GetID("luaddock");
  ImGui::DockSpace(dockid, ImVec2(0.0f, 0.0f));

  render_menubar();

  ImGui::End();

  for (wininfo &wi: decompilers_info) {
    if (!wi.view)
      continue;
    ImGui::SetNextWindowDockID(dockid, ImGuiCond_FirstUseEver);
    ImGui::Begin(wi.name.c_str());
    wi.func(this);
    ImGui::End();
  }

  for (wininfo &wi: plugins_info) {
    if (!wi.view)
      continue;
    ImGui::SetNextWindowDockID(dockid, ImGuiCond_FirstUseEver);
    ImGui::Begin(wi.name.c_str());
    wi.func(this);
    ImGui::End();
  }
}
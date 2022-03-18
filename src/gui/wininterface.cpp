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

#include "../main.hpp"
#include "../os/convert.hpp"

#include <chrono>

#include <fmt/xchar.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"

#include "nfd.hpp"

#include "wininterface.hpp"

#include "../imgui_addons/imgui_notf.hpp"
#include "../imgui_addons/imgui_addons.hpp"

using namespace std::chrono_literals;

namespace luad {
wininterface::wininterface() : glfw::Window{800, 600, "Luad"} {
  glfw::makeContextCurrent(*this);
  ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow *>(*this), true);
  ImGui::StyleColorsDark();

  update_settings();

  keyEvent.setCallback(
      [this](glfw::Window &, glfw::KeyCode key, int scan, glfw::KeyState state, glfw::ModifierKeyBit modifiers) {
        auto it = std::find_if(shortcuts.begin(), shortcuts.end(), [modifiers, key, state](const shortcut &s) {
          bool res = s.mod == glfw::ModifierKeyBit{} || modifiers & s.mod;
          return res && key == s.key && state == s.state;
        });
        if (it != shortcuts.end())
          it->func();
      });

  maximizeEvent.setCallback([this](glfw::Window &, bool status) {
    setting.maximized(status);
  });

  sizeEvent.setCallback([this](glfw::Window &, int width, int height) {
    setting.size(width, height);
  });

  posEvent.setCallback([this](glfw::Window &, int x, int y) {
    setting.pos(x, y);
  });

  shortcuts.push_back({
      .mod = glfw::ModifierKeyBit::Control,
      .key = glfw::KeyCode::O,
      .func = [this] { open_filedialog(); }
  });
  shortcuts.push_back({
      .mod = glfw::ModifierKeyBit::Control,
      .key = glfw::KeyCode::G,
      .func = [this] { if (file) disasm.enable_goto_dialog(); }
  });
//  shortcuts.push_back({
//      .mod = glfw::ModifierKeyBit::Control,
//      .key = glfw::KeyCode::R,
//      .func = [this]() {
//        setting.save();
//        update_settings();
//      }
//  });
}

wininterface::~wininterface() {
  setting.save();
}

void wininterface::update_settings() {
  setting.load();

  ImGuiIO &io = ImGui::GetIO();
  const auto [x, y] = setting.pos();
  const auto [width, height] = setting.size();
  setPos(x, y);
  setSize(width, height);

  if (setting.maximized())
    maximize();

  const auto [fontpath, fontsize] = setting.font();
  io.Fonts->ClearFonts();
  constexpr ImWchar ranges[] = {
      0x0020, 0x00FF,
      0x0400, 0x052F,
      0x2DE0, 0x2DFF,
      0xA640, 0xA69F,
      0x2013, 0x2122,
      0};
  const ImFont *font = io.Fonts->AddFontFromFileTTF(expand_environment(fontpath).c_str(), fontsize, nullptr, ranges);
  if (font == nullptr)
    return;
  io.Fonts->Build();
}

void wininterface::open_file(const std::filesystem::path &p) {
  file = std::make_shared<luac_file>(p);
  if (file->error() == luac_file::errors::no) {
#ifdef LUAD_WINDOWS
    std::wstring filename = p.filename().wstring();
    std::wstring wname_window = fmt::format(L"Luad: {}", filename);
    std::string name_window = from_widechar(wname_window);
#else
    std::string filename = p.filename().string();
    std::string name_window = fmt::format("Luad: {}", filename);
#endif
    setTitle(name_window.c_str());

    disasm.update(file, setting.bcoptions());
    return;
  }

  std::string error = p.filename().string() + ": ";
  switch (file->error()) {
  case luac_file::errors::is_file:
    error += "The path isn't a file or a file doesn't exist.";
    break;
  case luac_file::errors::open_file:
    error += "Error opening file.";
    break;
  case luac_file::errors::parse:
    error += "Unknown compiler of lua script.";
    break;
  default:
    break;
  }

  ImGuiNotf::Add(error, 3000ms, ImGuiNotf::ErrorStyle());
  close_file();
}

void wininterface::open_filedialog() {
  if (file)
    close_file();

  NFD::UniquePath out;
  constexpr nfdfilteritem_t filter[] = {{"Compiled lua script", "luac"}};
  const nfdresult_t result = NFD::OpenDialog(out, filter, 1);
  if (result != NFD_OKAY)
    return;

#ifdef LUAD_WINDOWS
  open_file(to_widechar(out.get()));
#else
  open_file(out.get());
#endif
}

void wininterface::close_file() {
  disasm.reset();
  file.reset();

  setTitle("Luad");
}

void wininterface::render_popup() {
  if (open_popup) {
    ImGui::OpenPopup(open_popup);
    ImVec2 winsize = ImGui::GetMainViewport()->WorkSize;
    winsize.x /= 2.f;
    winsize.y /= 2.f;
    ImGui::SetNextWindowPos(winsize, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    open_popup = nullptr;
  }

  if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
    ImGui::Text("Luad - Disassembler for compiled Lua scripts.");
    ImGui::Text("Version: %ld.%02ld.", LUAD_VERSION / 100, LUAD_VERSION % 100);
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

void wininterface::render_menubar() {
  if (!ImGui::BeginMenuBar())
    return;

  if (ImGui::BeginMenu("File")) {
    if (ImGui::MenuItem("Open", "Ctrl+O"))
      open_filedialog();
    if (ImGui::MenuItem("Close", nullptr, nullptr, static_cast<bool>(file)))
      close_file();
    // ImGui::MenuItem("Save", "Ctrl+S", nullptr, static_cast<bool>(file));
    ImGui::Separator();

    if (ImGui::MenuItem("Settings")) {
      setting.winactive = true;
      ImGui::SetWindowFocus("Settings");
    }

    ImGui::Separator();
    if (ImGui::MenuItem("Quit"))
      setShouldClose(true);
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("View", static_cast<bool>(file))) {
    if (ImGui::BeginMenu("Disassembly")) {
      disasm.render_menu();
      ImGui::EndMenu();
    }
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Help")) {
    if (ImGui::MenuItem("About"))
      open_popup = "About";
    ImGui::EndMenu();
  }

  ImGui::EndMenuBar();
}

void wininterface::render() {
  const ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                  ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                  ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

  ImGui::Begin("Luad", nullptr, window_flags);
  ImGui::PopStyleVar(3);

  const ImGuiID dockid = ImGui::GetID("luaddock");
  ImGui::DockSpace(dockid, ImVec2{});
  render_menubar();
  ImGui::End();


  disasm.render();
  if (setting.winactive) {
    ImGui::SetNextWindowDockID(dockid, ImGuiCond_FirstUseEver);
    setting.render();
  }

  render_popup();
  ImGuiNotf::Render();
}
} // namespace luad
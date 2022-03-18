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

#ifndef LUAD_WININTERFACE_H
#define LUAD_WININTERFACE_H

#include <nlohmann/json.hpp>

#include "glfwpp/glfwpp.h"
#include "imgui_memory_editor.h"
#include "TextEditor.h"

#include "../file.hpp"
#include "settings.hpp"
#include "bclist.hpp"
#include "disassembly.hpp"

namespace luad {
class wininterface : public glfw::Window {
  struct shortcut {
    glfw::ModifierKeyBit mod = {};
    glfw::KeyCode key = {};
    glfw::KeyState state = glfw::KeyState::Press;
    std::function<void()> func = {};
  };

  std::shared_ptr<luac_file> file;
  std::vector<shortcut> shortcuts;

  disassembly disasm;
  settings setting;

  const char *open_popup = nullptr;

  void update_settings();

  void open_filedialog();
  void close_file();

  void render_popup();
  void render_menubar();

public:
  wininterface();
  ~wininterface();

  void open_file(const std::filesystem::path &p);
  void render();
};
} // namespace luad

#endif // LUAD_WININTERFACE_H

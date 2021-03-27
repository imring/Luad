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

#include <bitset>
#include <string>

#include "bclist.hpp"
#include "window.hpp"

#include "TextEditor.h"
#include "imgui.h"

#include "fmt/os.h"

void luad::lj::window::render_lj_dissasembly(wininterface *win) {
  win->editor.Render("TextEditor");
}

void luad::lj::window::add_info() {
  auto lang = TextEditor::LanguageDefinition::Lua();

  // vscode-like style
  constexpr TextEditor::Palette p = {
      0xffd4d4d4, // Default
      0xff569cd6, // Keyword
      0xff7fb347, // Number
      0xff7891ce, // String
      0xff7891ce, // Char literal
      0xffffffff, // Punctuation
      0xffd197d9, // Preprocessor
      0xffaaaaaa, // Identifier
      0xff9bc64d, // Known identifier
      0xffc040a0, // Preproc identifier
      0xff608b4e, // Comment (single line)
      0xff608b4e, // Comment (multi line)
      0xff1e1e1e, // Background
      0xffe0e0e0, // Cursor
      0x80a06020, // Selection
      0x800020ff, // ErrorMarker
      0x40f08000, // Breakpoint
      0xffa0a0a0, // Line number
      0x002a2a2a, // Current line fill
      0x402a2a2a, // Current line fill (inactive)
      0x40aaaaaa, // Current line edge
  };

  editor.SetShowWhitespaces(false);
  editor.SetPalette(p);
  editor.SetReadOnly(true);
  editor.SetTabSize(4);
  editor.SetLanguageDefinition(lang);
  editor.SetText(bc.full());

  decompilers_info.push_back({"Dissasembly", render_lj_dissasembly});
}

std::string trim(std::string_view s) {
  auto first = s.begin(), last = s.end() - 1;
  for (; std::isspace(*first); first++);
  for (; std::isspace(*last); last--);
  return std::string(first, last + 1);
}

bool imgui_selectable_modified(const char *label, bool &is_opened) {
  ImGuiStorage *storage = ImGui::GetStateStorage();
  bool res = ImGui::Selectable(label);

  ImGuiID id = ImGui::GetID(label);
  is_opened = storage->GetInt(id) != 0;
  if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    ImGui::GetStateStorage()->SetInt(id, is_opened ? 0 : 1);
  return res;
}

std::ptrdiff_t find_line(std::vector<std::string> text, std::string s,
                         std::ptrdiff_t start_line = 0) {
  auto begin = text.begin(), end = text.end();

  auto it = std::find(begin + start_line, end, s);
  if (it != end)
    return std::distance(begin, it);
  return PTRDIFF_MAX;
}

void luad::lj::window::render_left_panel() {
  ImGui::Text("Prototypes:");
  ImGui::Separator();
  for (size_t l = 2, i = 0; l < bc.divs.size(); ++i, ++l) {
    std::string proto_str = "proto" + std::to_string(i);
    std::vector<std::string> text = editor.GetTextLines();

    bool is_opened = false;
    if (imgui_selectable_modified(proto_str.c_str(), is_opened)) {
      std::ptrdiff_t line = find_line(text, proto_str + " do");

      if (line != PTRDIFF_MAX) {
        editor.SetCursorPosition(
            TextEditor::Coordinates(static_cast<int>(line), 0));
      }
    }
    if (!is_opened)
      continue;

    bclist::div proto_div = bc.divs[l];
    for (auto add: proto_div.additional) {
      if (add.header.empty())
        continue;

      std::string trim_header = trim(add.header);
      std::string add_info = fmt::format(" {}##{}", trim(add.header), i);
      if (!ImGui::Selectable(add_info.c_str()))
        continue;

      std::ptrdiff_t proto_line = find_line(text, proto_str + " do");
      if (proto_line == PTRDIFF_MAX)
        continue;
          
      std::ptrdiff_t line = find_line(text, "\t" + trim_header, proto_line);
      if (line != PTRDIFF_MAX)
        editor.SetCursorPosition(
            TextEditor::Coordinates(static_cast<int>(line), 1));
    }
  }
}
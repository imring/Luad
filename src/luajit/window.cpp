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

void luad::lj::window::render_lj_dissasembly(wininterface *win) {
  win->editor.Render("TextEditor");
}

void luad::lj::window::add_info() {
  bclist bc(info());
  auto lang = TextEditor::LanguageDefinition::Lua();

  // vscode-like style
  static TextEditor::Palette p = {{
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
  }};

  editor.SetShowWhitespaces(false);
  editor.SetPalette(p);
  editor.SetReadOnly(true);
  editor.SetTabSize(4);
  editor.SetLanguageDefinition(lang);
  editor.SetText(bc.full());

  decompilers_info.push_back({"Dissasembly", render_lj_dissasembly});
}
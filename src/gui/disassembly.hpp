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

#ifndef LUAD_DISASSEMBLY_H
#define LUAD_DISASSEMBLY_H

#include "TextEditor.h"
#include "imgui_memory_editor.h"

#include "bclist.hpp"
#include "../file.hpp"

namespace luad {
class disassembly {
  std::weak_ptr<luac_file> file;
  std::unique_ptr<bclist> ptr;
  bclist::div lines;

  struct protos_t {
    bool winactive = true;

    void render(disassembly &disasm);
  } protos;
  struct editor_t : public TextEditor {
    bool winactive = true;
  } editor;
  struct goto_t {
    int addr = 0;
    bool winactive = false;

    void render(disassembly &disasm);
  } goto_dialog;
  MemoryEditor hex_editor;

  bool go_to(size_t addr);

public:
  disassembly();

  void update(std::weak_ptr<luac_file> f, const bclist::options &op = bclist::options{});
  void reset();

  void render_menu();
  void render();

  void enable_goto_dialog() { goto_dialog.winactive = true; }
};
} // namespace luad

#endif // LUAD_DISASSEMBLY_H

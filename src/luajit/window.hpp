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

#ifndef LUAD_LUAJIT_WINDOW_H
#define LUAD_LUAJIT_WINDOW_H

#include <filesystem>
#include <iostream>

#include "wininterface.hpp"

namespace luad::lj {
class window : public wininterface {
public:
  window(const std::filesystem::path &p) : wininterface(p), prototype_n(0) {
    add_info();
  }
  window(luac_file &file) : wininterface(file), prototype_n(0) { add_info(); }
  ~window() {}

  void add_info();
  // void render();

protected:
  dislua::uint prototype_n;

private:
  static void render_lj_information(wininterface *win);
  static void render_lj_dissasembly(wininterface *win);
};
} // namespace luad::lj

#endif // LUAD_LUAJIT_WINDOW_H
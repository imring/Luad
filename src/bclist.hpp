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

#ifndef BCLIST_H
#define BCLIST_H

#include <memory>

#include "dislua/dislua.hpp"

class bclist {
public:
  bclist(dislua::dump_info *i) : info(i) { update(); }

  struct div {
    std::string header, lines, footer;
    std::vector<div> additional;
  };

  std::string full();
  void update();

  std::vector<div> divs;
  dislua::dump_info *info;

private:
  std::string lj_full();
  void lj_update();
};

#endif // BCLIST_H
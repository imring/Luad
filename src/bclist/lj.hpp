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

#ifndef BCLIST_LJ_H
#define BCLIST_LJ_H

#include "bclist.hpp"

class bclist_lj : public bclist {
public:
  bclist_lj(dislua::dump_info *i) : bclist(i) { update(); }

  void update();

private:
  void update_string();
  div proto(size_t idx);
};

#endif // BCLIST_LJ_H
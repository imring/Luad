// Luad - Disassembler for compiled Lua scripts.
// https://github.com/imring/Luad
// Copyright (C) 2021-2023 Vitaliy Vorobets
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

#ifndef LUAD_CUSTOMFUNCS_HPP
#define LUAD_CUSTOMFUNCS_HPP

#include <sol/sol.hpp>

#include "plugins.hpp"

namespace LuaCustom {
void initialize_dislua_types(sol::state &lua);
void initialize_bclist_types(sol::state &lua);
void initialize(LuaPlugin &plugin);

void print(LuaPlugin &plugin, const sol::variadic_args &args);
void highlight(int from, int to, int color);
// todo:
// jump, highlight, addresses/lines/variables/bytes
// on open file events
}

#endif // LUAD_CUSTOMFUNCS_HPP
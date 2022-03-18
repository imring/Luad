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

#ifndef IMGUI_ADDONS_H
#define IMGUI_ADDONS_H

#include <string_view>

namespace ImGui {
bool SelectableDetail(const char *label, bool &is_opened);
bool InputTextWithReset(std::string_view label, char *buf, size_t buf_size, std::string_view default_buf);
bool InputFloatWithReset(std::string_view label, float &val, float default_val, float step = 1.0f,
                         const char *format = "%.3f");
bool InputIntWithReset(std::string_view label, int &val, int default_val, int step = 1);
}

#endif // IMGUI_ADDONS_H
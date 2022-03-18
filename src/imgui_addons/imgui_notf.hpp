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

#ifndef IMGUI_NOTF_H
#define IMGUI_NOTF_H

#include <chrono>

#include <imgui.h>

namespace ImGuiNotf {
struct Style {
  ImVec4 text;
  ImVec4 background;
};

void Add(std::string_view text, std::chrono::milliseconds ms, const Style &style);
void Render();

Style NormalStyle();
Style InfoStyle();
Style ErrorStyle();

inline struct {
  ImVec2 padding = {10.f, 10.f};
  ImVec2 size = {200.f, 0.f};
  float animspeed = 0.3f;
} settings;
} // namespace ImGuiNotf

#endif // IMGUI_NOTF_H
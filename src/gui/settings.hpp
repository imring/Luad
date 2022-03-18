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

#ifndef LUAD_SETTINGS_H
#define LUAD_SETTINGS_H

#include <nlohmann/json.hpp>

#include "bclist.hpp"

namespace luad {
struct settings {
private:
  enum class menu_items {
    window,
    bclist
  };
  menu_items selected = menu_items::window;

  static nlohmann::json standart();
  void render_mi_window();
  void render_mi_bclist();
public:
  std::filesystem::path path = "./config.json";
  nlohmann::json config;
  bool winactive = false;

  void load();
  void save() const;
  void render();

  std::pair<int, int> size();
  void size(int x, int y);

  std::pair<int, int> pos();
  void pos(int width, int height);

  bool maximized();
  void maximized(bool val);

  std::pair<std::string, float> font();
  void font(std::string_view p, float size);

  bclist::options bcoptions();
  void bcoptions(const bclist::options &op);
};
} // namespace luad

#endif // LUAD_SETTINGS_H
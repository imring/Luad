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

#include <fstream>

#include "imgui.h"

#include "settings.hpp"
#include "../imgui_addons/imgui_addons.hpp"

template <typename T, typename... Args>
void normalize(nlohmann::json &from, const nlohmann::json &to, T &&index, Args&&... args) {
  using index_t = std::string_view;

  if constexpr (std::is_same_v<T, std::initializer_list<index_t>>) {
    for (const index_t &v: index)
      normalize(from, to, v, std::forward<Args>(args)...);
  } else {
    const std::string sindex{index};
    if (!from.contains(index)) {
      from[sindex] = to[sindex];
      return;
    }

    if constexpr (sizeof...(args) > 0)
      normalize(from[sindex], to[sindex], std::forward<Args>(args)...);
  }
}

nlohmann::json luad::settings::standart()  {
  static const nlohmann::json j = R"(
{
  "win": {
    "x": 40,
    "y": 40,
    "width": 800,
    "height": 600,
    "maximized": false,

    "font": {
      "path": "./fonts/LiberationMono-Regular.ttf",
      "size": 13
    }
  },

  "bclist": {
    "max_length": 50
  }
}
)"_json;
  return j;
}

void luad::settings::load() {
  std::ifstream file{path};
  if (!file) {
    config = standart();
    return;
  }
  file >> config;
  file.close();
}

void luad::settings::save() const {
  std::ofstream file{path};
  if (!file)
    return;

  file << std::setw(4) << config;
  file.close();
}

void luad::settings::render_mi_window() {
  // Font
  auto &&[fpath, fsize] = font();
  fpath.resize(260);
  const nlohmann::json fontstd = standart()["win"]["font"];
  const std::string fpathstd = fontstd["path"].get<std::string>();
  const float fsizestd = fontstd["size"].get<float>();

  ImGui::Text("Font");
  ImGui::Separator();

  ImGui::TextWrapped("Path to the font file. Environment variables can be used.");
  if (ImGui::InputTextWithReset("##fpath", fpath.data(), fpath.size(), fpathstd)) {
    fpath.erase(fpath.find('\0'));
    font(fpath, fsize);
  }
  ImGui::NewLine();

  ImGui::TextWrapped("Font size.");
  if (ImGui::InputFloatWithReset("##fsize", fsize, fsizestd, 1.f, "%.0f"))
    font(fpath, fsize);
}

void luad::settings::render_mi_bclist() {
  bclist::options op = bcoptions();
  const nlohmann::json bcliststd = standart()["bclist"];
  const int max_lengthstd = bcliststd["max_length"].get<int>();

  ImGui::TextWrapped("Maximum length of strings/tables without line break.");
  int imaxlen = static_cast<int>(op.max_length);
  if (ImGui::InputIntWithReset("##bcmax_length", imaxlen, max_lengthstd)) {
    op.max_length = static_cast<size_t>(imaxlen);
    bcoptions(op);
  }
}

void luad::settings::render() {
  ImGui::Begin("Settings", &winactive);

  const float width = ImGui::GetWindowWidth() / 4.f;
  ImGui::BeginChild("##setmenu", ImVec2{width, 0}, true);
  if (ImGui::Selectable("Window", selected == menu_items::window))
    selected = menu_items::window;
  ImGui::Separator();
  if (ImGui::Selectable("bclist", selected == menu_items::bclist))
    selected = menu_items::bclist;
  ImGui::EndChild();
  ImGui::SameLine();

  ImGui::BeginChild("##infomenu");
  switch (selected) {
  case menu_items::window:
    render_mi_window();
    break;
  case menu_items::bclist:
    render_mi_bclist();
    break;
  }
  ImGui::EndChild();

  ImGui::End();
}

std::pair<int, int> luad::settings::size() {
  normalize(config, standart(), "win", std::initializer_list<std::string_view>{ "width", "height" });

  const nlohmann::json &confwin = config["win"];
  return { confwin["width"].get<int>(), confwin["height"].get<int>() };
}

void luad::settings::size(int width, int height) {
  normalize(config, standart(), "win");

  nlohmann::json &confwin = config["win"];
  confwin["width"] = width;
  confwin["height"] = height;
}

std::pair<int, int> luad::settings::pos() {
  normalize(config, standart(), "win", std::initializer_list<std::string_view>{ "x", "y" });

  const nlohmann::json &confwin = config["win"];
  return { confwin["x"].get<int>(), confwin["y"].get<int>() };
}

void luad::settings::pos(int x, int y) {
  normalize(config, standart(), "win");

  nlohmann::json &confwin = config["win"];
  confwin["x"] = x;
  confwin["y"] = y;
}

bool luad::settings::maximized() {
  normalize(config, standart(), "win", "maximized");
  return config["win"]["maximized"].get<bool>();
}

void luad::settings::maximized(bool val) {
  normalize(config, standart(), "win");

  nlohmann::json &confwin = config["win"];
  confwin["maximized"] = val;
}

std::pair<std::string, float> luad::settings::font() {
  normalize(config, standart(), "win", "font", std::initializer_list<std::string_view>{ "path", "size" });

  const nlohmann::json &conffont = config["win"]["font"];
  return { conffont["path"].get<std::string>(), conffont["size"].get<float>() };
}

void luad::settings::font(std::string_view p, float size) {
  normalize(config, standart(), "win", "font");

  nlohmann::json &conffont = config["win"]["font"];
  conffont["path"] = p;
  conffont["size"] = size;
}

bclist::options luad::settings::bcoptions() {
  normalize(config, standart(), "bclist");
  return bclist::options{config["bclist"]["max_length"].get<size_t>()};
}

void luad::settings::bcoptions(const bclist::options &op) {
  config["bclist"]["max_length"] = op.max_length;
}
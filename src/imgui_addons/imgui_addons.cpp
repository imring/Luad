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

#include <string>
#include <cstring>

#include <imgui.h>

#include "imgui_addons.hpp"

bool ImGui::SelectableDetail(const char *label, bool &is_opened) {
  const ImGuiStorage *storage = ImGui::GetStateStorage();
  const bool res = ImGui::Selectable(label);

  ImGuiID id = ImGui::GetID(label);
  is_opened = storage->GetInt(id) != 0;
  if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    ImGui::GetStateStorage()->SetInt(id, is_opened ? 0 : 1);
  return res;
}

bool ImGui::InputTextWithReset(std::string_view label, char *buf, size_t buf_size, std::string_view default_buf) {
  const ImGuiStyle &style = ImGui::GetStyle();
  constexpr std::string_view reset_name = "Reset";

  const bool original = std::string_view{buf} == default_buf;
  const float button_size = !original
    ? ImGui::CalcTextSize(reset_name.data()).x + style.FramePadding.x * 2.f + style.ItemSpacing.x
    : 0;

  ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - button_size);
  bool res = ImGui::InputText(label.data(), buf, buf_size);
  ImGui::PopItemWidth();
  ImGui::SameLine();

  if (!original) {
    std::string button_label{std::string{reset_name} + "##" + std::string{label}};
    if (ImGui::Button(button_label.c_str())) {
      std::strcpy(buf, default_buf.data());
      res = true;
    }
  }

  return res;
}

bool ImGui::InputFloatWithReset(std::string_view label, float &val, float default_val, float step, const char *format) {
  const ImGuiStyle &style = ImGui::GetStyle();
  constexpr std::string_view reset_name = "Reset";

  const bool original = val == default_val;
  const float button_size = !original
    ? ImGui::CalcTextSize(reset_name.data()).x + style.FramePadding.x * 2.f + style.ItemSpacing.x
    : 0;

  ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - button_size);
  bool res = ImGui::InputFloat(label.data(), &val, step, step, format);
  ImGui::PopItemWidth();
  ImGui::SameLine();

  if (!original) {
    std::string button_label{std::string{reset_name} + "##" + std::string{label}};
    if (ImGui::Button(button_label.c_str())) {
      val = default_val;
      res = true;
    }
  }

  return res;
}

bool ImGui::InputIntWithReset(std::string_view label, int &val, int default_val, int step) {
  const ImGuiStyle &style = ImGui::GetStyle();
  constexpr std::string_view reset_name = "Reset";

  const bool original = val == default_val;
  const float button_size = !original
    ? ImGui::CalcTextSize(reset_name.data()).x + style.FramePadding.x * 2.f + style.ItemSpacing.x
    : 0;

  ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - button_size);
  bool res = ImGui::InputInt(label.data(), &val, step, step);
  ImGui::PopItemWidth();
  ImGui::SameLine();

  if (!original) {
    std::string button_label{std::string{reset_name} + "##" + std::string{label}};
    if (ImGui::Button(button_label.c_str())) {
      val = default_val;
      res = true;
    }
  }

  return res;
}
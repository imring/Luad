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

#define IMGUI_DEFINE_MATH_OPERATORS

#include <chrono>
#include <string>
#include <vector>

#include <imgui_internal.h>

#include "imgui_notf.hpp"

using namespace std::chrono;

struct NotfMessage {
  milliseconds endtime;
  std::string text;
  ImGuiNotf::Style style;
  float alpha = 1.f;

  NotfMessage(std::string_view text_, milliseconds time_, const ImGuiNotf::Style &style_)
      : endtime{time_ + duration_cast<milliseconds>(system_clock::now().time_since_epoch())}, text{text_},
        style{style_}, alpha{1.f} {}
};

std::vector<NotfMessage> messages;

void ImGuiNotf::Add(std::string_view text, milliseconds ms, const ImGuiNotf::Style &style) {
  messages.emplace_back(text, ms, style);
}

void ImGuiNotf::Render() {
  const ImGuiStyle &style = ImGui::GetStyle();

  const std::string prefix = "##notf";
  constexpr int flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

  milliseconds current_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
  float height = 0.f;

  for (auto it = messages.begin(); it != messages.end();) {
    size_t i = messages.begin() - it;

    NotfMessage &msg = *it;
    if (msg.endtime < current_time) {
      const auto m = duration_cast<duration<float>>(current_time - msg.endtime);
      msg.alpha = 1.f - ImSaturate(m.count() / settings.animspeed);

      if (msg.alpha <= 0.f) {
        it = messages.erase(it);
        continue;
      }
    }

    ImGui::SetNextWindowBgAlpha(msg.alpha);

    ImVec2 pos = ImGui::GetMainViewport()->Size - settings.padding;
    pos.y -= height;
    ImGui::SetNextWindowPos(pos, ImGuiCond_None, {1.f, 1.f});
    ImGui::SetNextWindowSize(settings.size);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, msg.style.background);
    ImGui::Begin((prefix + std::to_string(i)).c_str(), nullptr, flags);

    ImGui::PushTextWrapPos(settings.size.x);
    ImGui::PushStyleColor(ImGuiCol_Text, msg.style.text);
    ImGui::TextUnformatted(msg.text.c_str());
    ImGui::PopStyleColor();
    ImGui::PopTextWrapPos();

    height += ImGui::GetWindowHeight() + style.FramePadding.y;
    ImGui::End();
    ImGui::PopStyleColor();

    it++;
  }
}

ImGuiNotf::Style ImGuiNotf::NormalStyle() {
  return {
    ImVec4{1.f, 1.f, 1.f, 1.f},
    ImVec4{0.f, 0.f, 0.f, 1.f},
//    {
//      ImVec4{0.f, 0.f, 0.f, 0.66f},
//      ImVec4{0.f, 0.f, 0.f, 0.92f},
//      ImVec4{0.f, 0.f, 0.f, 0.92f},
//      ImVec4{0.f, 0.f, 0.f, 0.66f}
//    }
  };
}

ImGuiNotf::Style ImGuiNotf::InfoStyle() {
  return {
    ImVec4{1.f, 1.f, 1.f, 1.f},
    ImVec4{0.f, 0.62f, 0.76f, 1.f},
//    {
//      ImVec4{0.f, 0.54f, 0.68f, 0.66f},
//      ImVec4{0.f, 0.54f, 0.68f, 0.92f},
//      ImVec4{0.f, 0.54f, 0.68f, 0.92f},
//      ImVec4{0.f, 0.54f, 0.68f, 0.66f}
//    }
  };
}

ImGuiNotf::Style ImGuiNotf::ErrorStyle() {
  return {
    ImVec4{1.f, 1.f, 1.f, 1.f},
    ImVec4{0.75f, 0.16f, 0.16f, 1.f},
//    {
//      ImVec4{0.66f, 0.08f, 0.08f, 0.66f},
//      ImVec4{0.66f, 0.08f, 0.08f, 0.92f},
//      ImVec4{0.66f, 0.08f, 0.08f, 0.92f},
//      ImVec4{0.66f, 0.08f, 0.08f, 0.66f}
//    }
  };
}
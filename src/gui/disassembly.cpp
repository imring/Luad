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

#include <chrono>

#include "disassembly.hpp"

#include "../imgui_addons/imgui_notf.hpp"
#include "../imgui_addons/imgui_addons.hpp"

using namespace std::chrono_literals;

const TextEditor::LanguageDefinition &lang_def() {
  static bool init = false;
  static TextEditor::LanguageDefinition def;
  if (!init) {
    static constexpr std::string_view keywords[] = {
        "and", "break", "do",  "",    "else", "elseif", "end",    "false", "for",  "function", "if",   "in",
        "",    "local", "nil", "not", "or",   "repeat", "return", "then",  "true", "until",    "while"};

    for (auto &k: keywords)
      def.mKeywords.emplace(k);

    def.mIdentifiers.emplace("invalid", TextEditor::Identifier{.mDeclaration = "Invalid value in the instruction"});

    def.mTokenRegexStrings.emplace_back("L?\\\"(\\\\.|[^\\\"])*\\\"", TextEditor::PaletteIndex::String);
    def.mTokenRegexStrings.emplace_back("\\\'[^\\\']*\\\'", TextEditor::PaletteIndex::String);

    def.mTokenRegexStrings.emplace_back("0[bB][01]+", TextEditor::PaletteIndex::Number);
    def.mTokenRegexStrings.emplace_back("[0-9a-fA-F]{8}", TextEditor::PaletteIndex::Number);
    def.mTokenRegexStrings.emplace_back("0[xX][0-9a-fA-F]+", TextEditor::PaletteIndex::Number);
    def.mTokenRegexStrings.emplace_back("([0-9a-fA-F]{2}\\s?){4}", TextEditor::PaletteIndex::Number);
    def.mTokenRegexStrings.emplace_back("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?",
                                            TextEditor::PaletteIndex::Number);
    def.mTokenRegexStrings.emplace_back("[+-]?[0-9]+", TextEditor::PaletteIndex::Number);

    def.mTokenRegexStrings.emplace_back("[a-zA-Z_][a-zA-Z0-9_]*", TextEditor::PaletteIndex::Identifier);
    def.mTokenRegexStrings.emplace_back("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]",
                                            TextEditor::PaletteIndex::Punctuation);

    def.mCommentStart = "--[[";
    def.mCommentEnd = "]]";
    def.mSingleLineComment = "--";
    def.mCaseSensitive = true;
    def.mAutoIndentation = false;
    def.mName = "bclist";

    init = true;
  }
  return def;
}

size_t line_by_addr(const bclist::div &div, size_t addr) {
  const bclist::div &only = div.only_lines();
  if (addr < only.start() || addr > only.end())
    return bclist::max_line;

  const auto binary_search = [](const bclist::div &div, size_t addr, size_t low, size_t high) {
    while (low <= high) {
      size_t mid = low + (high - low) / 2;
      const bclist::div::line &l = div.lines[mid];
      if (l.from <= addr && l.to >= addr)
        return mid;
      if (l.from < addr)
        low++;
      else
        high--;
    }
    return bclist::max_line;
  };

  size_t res = binary_search(only, addr, 0, only.lines.size());
  if (res != bclist::max_line) {
    while (res != 0 && div.lines[res - 1].from <= addr && div.lines[res - 1].to >= addr)
      res--;
  }
  return res;
}

namespace luad {
disassembly::disassembly() {
  // hex
  hex_editor.ReadOnly = true;

  // vscode-like style
  constexpr TextEditor::Palette p{
      0xffd4d4d4, // Default
      0xff569cd6, // Keyword
      0xff7fb347, // Number
      0xff7891ce, // String
      0xff7891ce, // Char literal
      0xffffffff, // Punctuation
      0xffd197d9, // Preprocessor
      0xffaaaaaa, // Identifier
      0xff9bc64d, // Known identifier
      0xffc040a0, // Preproc identifier
      0xff608b4e, // Comment (single line)
      0xff608b4e, // Comment (multi line)
      0xff1e1e1e, // Background
      0xffe0e0e0, // Cursor
      0x80a06020, // Selection
      0x800020ff, // ErrorMarker
      0x40f08000, // Breakpoint
      0xffa0a0a0, // Line number
      0x002a2a2a, // Current line fill
      0x402a2a2a, // Current line fill (inactive)
      0x40aaaaaa, // Current line edge
  };
  editor.SetLineNumberEnable(false);
  editor.SetShowWhitespaces(false);
  editor.SetPalette(p);
  editor.SetReadOnly(true);
  editor.SetTabSize(4);
  editor.SetLanguageDefinition(lang_def());
}

bool disassembly::go_to(size_t addr) {
  const size_t l = line_by_addr(lines, addr);
  if (l == bclist::max_line) {
    ImGuiNotf::Add(fmt::format("Invalid address: {:08X}", addr), 3000ms, ImGuiNotf::ErrorStyle());
    return false;
  } else {
    editor.SetCursorPosition(TextEditor::Coordinates{static_cast<int>(l), 0});
    return true;
  }
}

void disassembly::update(std::weak_ptr<luac_file> f, const bclist::options &op) {
  file = std::move(f);
  ptr = bclist::get_list(file.lock()->info());

  ptr->option = op;
  ptr->update();
  lines = ptr->divs.only_lines();

  const std::string str = lines.string();
  editor.SetColorizerEnable(str.size() < 1'000'000);
  editor.SetText(str);

  if (!editor.IsColorizerEnabled())
    ImGuiNotf::Add("Bytecode list is very large (>= 1m bytes), so syntax highlighting was disabled.", 3000ms,
                   ImGuiNotf::InfoStyle());
}

void disassembly::reset() {
  file.reset();
  ptr.reset();
  editor.SetText("");
}

void disassembly::render_menu() {
  ImGui::MenuItem("Hex editor", nullptr, &hex_editor.Open);
  ImGui::MenuItem("Prototypes", nullptr, &protos.winactive);
  ImGui::MenuItem("bclist", nullptr, &editor.winactive);
  ImGui::Separator();
  if (ImGui::MenuItem("Go to...", "Ctrl+G"))
    enable_goto_dialog();
}

void disassembly::protos_t::render(disassembly &disasm) {
  if (!winactive)
    return;

  ImGui::Begin("Prototypes", &winactive);

  for (const bclist::div &div: disasm.ptr->divs.additional) {
    std::string head = div.header;
    if (head.empty() || head.substr(0, 5) != "proto")
      continue;
    head = head.substr(0, head.size() - 3);

    bool is_opened = false;
    if (ImGui::SelectableDetail(head.c_str(), is_opened))
      disasm.go_to(div.start());

    if (!is_opened)
      continue;
    for (const bclist::div &add: div.additional) {
      const std::string add_head = " " + add.header + "##" + head;
      if (ImGui::Selectable(add_head.c_str()))
        disasm.go_to(add.start());
    }
  }

  ImGui::End();
}

void disassembly::goto_t::render(disassembly &disasm) {
  if (!winactive)
    return;

  const ImGuiStyle &style = ImGui::GetStyle();
  constexpr const char *title = "Go to...";
  if (winactive && !ImGui::IsPopupOpen(title))
    ImGui::OpenPopup(title);

  if (ImGui::BeginPopupModal(title, &winactive, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Enter an address: ");
    ImGui::InputInt("##gotoaddr", &addr, 0, 0, ImGuiInputTextFlags_CharsHexadecimal);

    constexpr float width = 100.f;
    bool result = ImGui::Button("OK", ImVec2{width, 0}) && disasm.go_to(addr);
    ImGui::SameLine();
    result = ImGui::Button("Cancel", ImVec2{width, 0}) || result;
    if (result) {
      addr = 0;
      winactive = false;
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
}

void disassembly::render() {
  const ImGuiID dockid = ImGui::GetID("luaddock");
  const std::shared_ptr<luac_file> f = file.lock();
  if (!f)
    return;

  // hex editor
  if (hex_editor.Open) {
    ImGui::SetNextWindowDockID(dockid, ImGuiCond_FirstUseEver);
    auto &&data = f->info()->buf.copy_data();
    hex_editor.DrawWindow("Hex editor", data.data(), data.size());
  }

  // bclist
  if (editor.winactive) {
    ImGui::SetNextWindowDockID(dockid, ImGuiCond_FirstUseEver);
    ImGui::Begin("bclist", &editor.winactive);
    editor.Render("bclist");
    ImGui::End();
  }

  ImGui::SetNextWindowDockID(dockid, ImGuiCond_FirstUseEver);
  protos.render(*this);
  ImGui::SetNextWindowDockID(dockid, ImGuiCond_FirstUseEver);
  goto_dialog.render(*this);
}
} // namespace luad
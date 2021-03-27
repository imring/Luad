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

#ifndef LUAD_WININTERFACE_H
#define LUAD_WININTERFACE_H

#include <functional>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "GLFW/glfw3.h"

#include "TextEditor.h"
#include "imgui.h"

#include "file.hpp"

namespace luad {
class wininterface : public luac_file {
public:
  struct wininfo {
    std::string name;
    std::function<void(wininterface *)> func;
  };

  wininterface(const std::filesystem::path &p) : luac_file(p) {}
  wininterface(luac_file &file) : luac_file(file) {}
  virtual ~wininterface() {}

  bool initialize();
  void render();
  inline ImFont *get_font(const std::string &v) { return fonts[v]; }
  ImVec2 get_window_size();

  // glfw functions
  inline void destroy() {
    if (win)
      glfwDestroyWindow(win);
  }
  inline void get_framebuf_size(int *display) {
    glfwGetFramebufferSize(win, &display[0], &display[1]);
  }
  inline bool should_close() { return glfwWindowShouldClose(win); }
  inline void set_should_close(bool v) {
    glfwSetWindowShouldClose(win, v ? 1 : 0);
  }
  inline void swap_buffers() { glfwSwapBuffers(win); }

  virtual void render_left_panel(){}

  TextEditor editor;

protected:
  std::vector<wininfo> decompilers_info;
  std::vector<wininfo> plugins_info;
  std::unordered_map<std::string, ImFont *> fonts;

private:
  void render_menubar();

  size_t selected_left = 0;
  GLFWwindow *win = nullptr;
};
} // namespace luad

#endif // LUAD_WININTERFACE_H
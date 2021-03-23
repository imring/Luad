# Luad
<p align="center"><img src="./assets/screen.png" height="500px" /></p>

[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/)
[![Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![License](https://img.shields.io/badge/license-GPL-blue.svg)](https://opensource.org/licenses/GPL-3.0)

Luad - Disassembler for compiled Lua scripts.  
At the moment the program is in development (v0.10-pre-alpha).

Supported compilers:
- [LuaJIT](http://luajit.org/) v1 & v2.

## Dependencies
- [GLFW](https://github.com/glfw/glfw);
- [gl3w](https://github.com/skaslev/gl3w);
- [Dear ImGui](https://github.com/ocornut/imgui) (has in a path `deps`);
- [ImGuiColorTextEdit](https://github.com/BalazsJako/ImGuiColorTextEdit) (has in a path `deps`);
- [{fmt}](https://github.com/fmtlib/fmt);
- [DisLua C++ (disluapp)](https://github.com/imring/disluapp).

## Build
```bash
$ git clone --recursive https://github.com/imring/luad
$ cd luad
$ mkdir build && cd build
$ cmake ..
$ cmake --build .
```
The build result will be in the directory `luad/output`.

## Using
Open the terminal and use the command:
```bash
./luad [file]
```

## TODO
- Graph view;
- View decompiled code;
- Writing your own Lua plugins;
- Rewrite the information;

## License
Luad is licensed under the [GNU General Public License v3.0](https://opensource.org/licenses/GPL-3.0).

## Copyright
[disluapp](https://github.com/imring/disluapp) is licensed under the [MIT License](https://opensource.org/licenses/MIT).  
[{fmt}](https://github.com/fmtlib/fmt) is licensed under the [MIT License](https://opensource.org/licenses/MIT).  
[ImGuiColorTextEdit](https://github.com/BalazsJako/ImGuiColorTextEdit) is licensed under the [MIT License](https://opensource.org/licenses/MIT).  
[Dear ImGui](https://github.com/ocornut/imgui) is licensed under the [MIT License](https://opensource.org/licenses/MIT).  
[GLFW](https://github.com/glfw/glfw) is licensed under the [zlib/libpng license](https://opensource.org/licenses/Zlib).  
[gl3w](https://github.com/skaslev/gl3w) is in the public domain.  
[OpenGL](https://www.opengl.org/) is a registered trademark of [Hewlett Packard Enterprise](https://www.hpe.com/).
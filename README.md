# Luad

Luad - Disassembler for compiled Lua scripts. At the moment the program is in development (v0.21-pre-alpha).

# Screenshot
![v0.21](./docs/assets/v0.21.png)

# Features

- [ ] Disassembly
    - [x] View pseudo-code;
    - [ ] Modify byte-code;
    - [x] Navigation with jumps/windows with functions or variables;
    - [x] Xrefs (external references);
    - [ ] Custom line highlight.
- [ ] Hex-editor
    - [ ] Link to Disassembly.
- [ ] Plugins

# Supported compilers
- [LuaJIT](https://luajit.org/) (v2.0 & v2.1);

## Build
To build it you'll need:
- Compiler with C++20 support;
- [CMake](https://cmake.org/);

You'll also need dependencies:
- [disluapp](https://github.com/imring/disluapp)*;
- [{fmt}](https://github.com/fmtlib/fmt)*;
- [Qt 6](https://www.qt.io/);

\* - is included in the project with [`FetchContent`](https://cmake.org/cmake/help/latest/module/FetchContent.html).

```shell
git clone https://github.com/imring/Luad
cd Luad
cmake . -B build
cmake --build build
```

## License
The program is licensed under the [GNU General Public License v3.0](LICENSE).
- disluapp is licensed under the [MIT License](https://github.com/imring/disluapp/blob/master/LICENSE).
- {fmt} is licensed under the MIT License.
- Qt is licensed under the [GNU Lesser General Public License (LGPL) v3.0](https://doc.qt.io/qt-6/lgpl.html).
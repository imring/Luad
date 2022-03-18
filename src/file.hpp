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

#ifndef LUAD_FILE_H
#define LUAD_FILE_H

#include <filesystem>

#include "dislua/dislua.hpp"

class luac_file {
public:
  enum class errors {
    no,
    is_file,
    open_file,
    parse
  };

  luac_file(luac_file &f) : e(f.e), _path(f._path), _info(std::move(f._info)) {}
  explicit luac_file(const std::filesystem::path &p);

  void write(bool verification = true);
  void save(bool verification = true);

  [[nodiscard]] errors error() const { return e; }
  [[nodiscard]] std::filesystem::path path() const { return _path; }
  [[nodiscard]] dislua::dump_info *info() const { return _info.get(); }

protected:
  errors e;
  std::filesystem::path _path;
  std::unique_ptr<dislua::dump_info> _info;
};

#endif // LUAD_FILE_H
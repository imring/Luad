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

#ifndef LUAD_FILE_H
#define LUAD_FILE_H

#include <filesystem>

#include "dislua/dislua.hpp"
namespace luad {
class luac_file {
public:
  enum errors { // LFE - Luac File Error
    LFE_NO,
    LFE_IS_FILE,
    LFE_OPEN_FILE,
    LFE_PARSE
  };

  luac_file(luac_file &f) : e(f.e), _path(f._path), _info(std::move(f._info)) {
    _info->buf = std::move(_info->buf);
  }
  luac_file(const std::filesystem::path &p);

  void save(bool verification = true);

  inline errors error() { return e; }
  inline std::filesystem::path path() { return _path; }
  inline dislua::dump_info *info() { return _info.get(); }

protected:
  errors e;
  std::filesystem::path _path;
  std::unique_ptr<dislua::dump_info> _info;
};
} // namespace luad

#endif // LUAD_FILE_H
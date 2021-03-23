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

#include <fstream>
#include <iterator>

#include "file.hpp"

luad::luac_file::luac_file(const std::filesystem::path &p)
    : e(LFE_NO), _path(p) {
  if (!std::filesystem::is_regular_file(p)) {
    e = LFE_IS_FILE;
    return;
  }

  std::ifstream luac(p, std::ios::binary);
  if (luac.fail()) {
    e = LFE_OPEN_FILE;
    return;
  }

  dislua::buffer buf((std::istreambuf_iterator<char>(luac)),
                     std::istreambuf_iterator<char>());
  _info = dislua::read_all(buf);
  if (!_info)
    e = LFE_PARSE;
  luac.close();
}

void luad::luac_file::save(bool verification) {
  if (verification) {
    std::unique_ptr<dislua::dump_info> cinfo;
    switch (_info->compiler()) {
    case dislua::DISLUA_LUAJIT:
      cinfo = std::make_unique<dislua::lj::parser>(*_info);
      break;
    default:
      cinfo = std::make_unique<dislua::dump_info>(*_info);
      break;
    }
    cinfo->write();
    cinfo->read();
  }

  _info->write();
  std::ofstream luac(_path, std::ios::binary);
  std::vector<dislua::uchar> buf = _info->buf->copy_data();
  std::copy(buf.begin(), buf.end(), std::ostream_iterator<char>(luac));
  luac.close();
}
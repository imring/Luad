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

#if !defined(LUAD_CONVERT_H) && defined(LUAD_UNIX)
#define LUAD_CONVERT_H

#include <string>
#include <string_view>

#include <wordexp.h>

inline std::string expand_environment(std::string_view str) {
  wordexp_t p;
  int result = wordexp(str.data(), &p, 0);
  if (result != 0)
    return {};

  std::string res;
  for (int i = 0; i < p.we_wordc; i++)
    res += std::string{p.we_wordv[i]} + ' ';
  res.pop_back();

  wordfree(&p);
  return res;
}

#endif // LUAD_CONVERT_H

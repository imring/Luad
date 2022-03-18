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

#ifndef LUAD_MAIN_H
#define LUAD_MAIN_H

#define LUAD_VERSION 20L

#if defined(unix) || defined(__unix__) || defined(__unix)
#  define LUAD_UNIX
#elif defined(_WIN32)
#  define LUAD_WINDOWS
#  if defined(UNICODE) || defined(_UNICODE)
#    define LUAD_WINUNICODE
#  endif
#endif

#endif // LUAD_MAIN_H
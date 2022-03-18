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

#if !defined(LUAD_CONVERT_H) && defined(LUAD_WINDOWS)
#define LUAD_CONVERT_H

#include <string>

#include <Windows.h>

inline std::string from_widechar(std::wstring_view s) {
  if (s.empty())
    return {};

  int size_needed = WideCharToMultiByte(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), NULL, 0, NULL, NULL);
  std::string result(static_cast<size_t>(size_needed), '\0');
  WideCharToMultiByte(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), result.data(), size_needed, NULL, NULL);
  return result;
}

inline std::wstring to_widechar(std::string_view s) {
  if (s.empty())
    return {};

  int size_needed = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), NULL, 0);
  std::wstring result(size_needed, '\0');
  MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), result.data(), size_needed);
  return result;
}

inline std::string expand_environment(std::string_view str) {
#ifdef LUAD_WINUNICODE
  std::wstring buf(MAX_PATH, '\0');
  DWORD result = ExpandEnvironmentStrings(to_widechar(str).data(), buf.data(), buf.size());
#else
  std::string buf(MAX_PATH, '\0');
  DWORD result = ExpandEnvironmentStrings(str.data(), buf.data(), buf.size());
#endif
  if (result == 0)
    return {};

#ifdef LUAD_WINUNICODE
  return from_widechar(buf);
#else
  return buf;
#endif
}

#endif // LUAD_CONVERT_H

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

#ifndef BCLIST_LJ_H
#define BCLIST_LJ_H

#include "bclist.hpp"

class bclist_lj : public bclist {
  std::vector<size_t> temp_protos_id;
  friend class bcproto_lj;

protected:
  static size_t uleb128_size(dislua::uleb128 val);
  static size_t uleb128_33_size(dislua::uleb128 val);
  static size_t uleb128_sizes(auto &&v);
  static size_t table_kv_size(const dislua::table_val_t &v);
  static size_t table_size(dislua::table_t t);

  [[nodiscard]] bool is_debug() const;
  [[nodiscard]] dislua::uchar bcmax() const;
  [[nodiscard]] auto bcopcode(size_t i) const;
  [[nodiscard]] int get_mode(dislua::uchar opcode) const;

  [[nodiscard]] std::string header_flags() const;
  [[nodiscard]] std::string fix_string(std::string_view str) const;
  [[nodiscard]] std::string varname(const dislua::varname &vn) const;
  [[nodiscard]] std::string table_kv(const dislua::table_val_t &v) const;
  std::string table(dislua::table_t t);

public:
  explicit bclist_lj(dislua::dump_info *i) : bclist(i) {}

  void update() override;
};

#endif // BCLIST_LJ_H
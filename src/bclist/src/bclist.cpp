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

#include <numeric>
#include <algorithm>
#include <functional>

#include <fmt/core.h>

#include "bclist.hpp"
#include "bclist/lj.hpp"

std::vector<std::string> split(const std::string &str, const char *delim) {
  std::vector<std::string> res;
  size_t prev_pos = 0, pos = 0;
  while ((pos = str.find(delim, pos)) != std::string::npos) {
    res.emplace_back(str.substr(prev_pos, pos - prev_pos));
    prev_pos = ++pos;
  }

  res.emplace_back(str.substr(prev_pos, pos - prev_pos));
  return res;
}

std::string bclist::div::string() const {
  const bclist::div &onl = only_lines();
  if (onl.lines.empty())
    return "";

//  too slow
//  std::string res = std::accumulate(
//      onl.lines.begin(), onl.lines.end(), std::string{},
//      [](const std::string &v, const bclist::div::line &l) {return v + fmt::format("{:08X}: {}\n", l.from, l); });

  std::string res;
  for (const bclist::div::line &l: onl.lines)
      res += fmt::format("{:08X}: {}\n", l.from, l);
  res.pop_back(); // remove last \n
  return res;
}

bclist::div bclist::div::only_lines() const {
  const bool already_done = header.empty() && footer.empty() && additional.empty() && tab == 0;
  if (already_done)
    return *this;

  const size_t st = start(), en = end();
  const std::string prev_tab(std::max(tab, size_t{1}) - 1, '\t'), cur_tab(tab, '\t');

  std::vector<bclist::div::line> res;
  static const auto add_line = [](std::vector<bclist::div::line> &ls, const bclist::div::line &l, const std::string &t) {
    for (const std::string &line: split(l, "\n"))
      ls.emplace_back(t + line, l.from, l.to);
  };

  if (!header.empty())
    add_line(res, bclist::div::line{header, st, st}, prev_tab);
  for (const bclist::div::line &line: lines)
    add_line(res, line, cur_tab);
  for (const div &add: additional) {
    const bclist::div &&onl = add.only_lines();
    for (const bclist::div::line &line: onl.lines)
      add_line(res, line, cur_tab);
  }
  if (!footer.empty())
    add_line(res, bclist::div::line{footer, en, en}, prev_tab);

  return bclist::div{.lines = res};
}

bool bclist::div::empty() const {
  if (!header.empty() || !footer.empty())
    return false;

  if (!lines.empty()) {
    bool em = std::all_of(lines.begin(), lines.end(), [](const std::string &v) {
      return v.empty();
    });
    if (!em)
      return false;
  }

  if (!additional.empty()) {
    bool em = std::all_of(additional.begin(), additional.end(), [](const div &v) {
      return v.empty();
    });
    if (!em)
      return false;
  }

  return true;
}

size_t bclist::div::start() const {
  if (!lines.empty())
    return lines.front().from;
  for (const auto &add: additional)
    if (const size_t res = add.start(); res != bclist::max_line)
      return res;
  return max_line;
}

size_t bclist::div::end() const {
  for (auto it = additional.rbegin(); it != additional.rend(); ++it)
    if (const size_t res = it->end(); res != bclist::max_line)
      return res;
  if (!lines.empty())
    return lines.back().from;
  return max_line;
}

void bclist::div::empty_line(size_t p) {
  if (p == bclist::max_line && !lines.empty())
    p = lines.back().from;
  new_line(p);
}

void bclist::div::add_div(const bclist::div &d) {
  if (!d.empty())
    additional.push_back(d);
}

std::unique_ptr<bclist> bclist::get_list(dislua::dump_info *info) {
  if (info->compiler() == dislua::compilers::luajit)
    return std::make_unique<bclist_lj>(info);
  return std::make_unique<bclist>(info);
}
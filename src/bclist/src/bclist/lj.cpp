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

#include <bitset>
#include <numeric>
#include <type_traits>

#include <dislua/const.hpp>
#include <fmt/core.h>

#include "lj.hpp"

namespace lj = dislua::lj;

template <typename T>
void add_flag(std::string &out, T flags) {
  out += std::to_string(static_cast<std::make_signed_t<T>>(flags));
  out += " | ";
}

template <typename T, typename U>
void add_flag(std::string &out, T &flags, U f, std::string_view name) {
  if ((flags & f) == 0)
    return;

  out += name;
  out += " | ";
  flags &= static_cast<T>(~f);
}

bool is_mode(int mode, int m, int shift = 0) {
  return ((mode >> shift) & lj::bcmode::MAX) == m;
}

bool has_b_field(int mode) { return !is_mode(mode, lj::bcmode::none, 3); }
bool is_jump(int mode) { return is_mode(mode, lj::bcmode::jump, 7); }

class bcproto_lj : public bclist_lj {
  size_t proto_id = 0;

public:
  explicit bcproto_lj(const bclist_lj &list, size_t _proto): bclist_lj{list}, proto_id{_proto} {}

  [[nodiscard]] static size_t knum_size(double val);
  [[nodiscard]] static size_t kgc_size(const dislua::kgc_t &v);

  [[nodiscard]] size_t header_size() const;
  [[nodiscard]] size_t ins_size() const;
  [[nodiscard]] size_t uv_size() const;
  [[nodiscard]] size_t kgc_size() const;
  [[nodiscard]] size_t knum_size() const;
  [[nodiscard]] size_t lineinfo_size() const;
  [[nodiscard]] size_t uvname_size() const;
  [[nodiscard]] size_t varname_size() const;

  static inline const std::string unkval = "invalid";

  [[nodiscard]] std::string get_uv(size_t i) const {
    if (i >= ref().uv.size())
      return unkval;

    return fmt::format("uv_{:d}_{:d}", proto_id, i);
  }
  [[nodiscard]] std::string get_pri(size_t i) const {
    static const std::string pri[] = { "nil", "false", "true" };

    if (i > 2)
      return unkval;
    return pri[i];
  }
  [[nodiscard]] std::string get_label(size_t i) const {
    if (i >= ref().ins.size())
      return unkval;

    return fmt::format("label_{:d}_{:d}", proto_id, i);
  }
  [[nodiscard]] std::string get_knum(size_t i) const {
    if (i >= ref().knum.size())
      return unkval;

    return fmt::format("knum_{:d}_{:d}", proto_id, i);
  }
  [[nodiscard]] std::string get_kgc(size_t i, dislua::uleb128 mode) const {
    if (i >= ref().kgc.size() || mode != ref().kgc[i].index())
      return unkval;

    return fmt::format("kgc_{:d}_{:d}", proto_id, i);
  }

  [[nodiscard]] dislua::proto &ref() const { return info->protos[proto_id]; }
  [[nodiscard]] std::string flags() const;
  [[nodiscard]] std::string fill_field(size_t i, int nfield) const;

  div ins();
  div uvdata();
  div kgc();
  div knum();
  // div varnames();
  div operator()();
};

size_t bclist_lj::uleb128_size(dislua::uleb128 val) {
  size_t res = 1;
  for (; val >= 0x80; val >>= 7, res++);
  return res;
}

size_t bclist_lj::uleb128_33_size(dislua::uleb128 val) {
  unsigned long long n = 1 + 2 * static_cast<unsigned long long>(val);
  size_t res = 1;
  for (; n >= 0x80; n >>= 7, res++);
  return res;
}

size_t bclist_lj::uleb128_sizes(auto &&v) {
  dislua::uleb128 arr[sizeof(v) / sizeof(dislua::uleb128)];
  std::memcpy(arr, &v, sizeof(v));

  size_t res = 0;
  for (const dislua::uleb128 val: arr)
    res += uleb128_size(val);
  return res;
}

size_t bclist_lj::table_kv_size(const dislua::table_val_t &v) {
  return std::visit(dislua::detail::overloaded{
    [](std::nullptr_t)     -> size_t { return 1; },
    [](bool)               -> size_t { return 1; },
    [](dislua::leb128 arg) -> size_t {
      return 1 + uleb128_size(static_cast<dislua::uleb128>(arg));
    },
    [](double arg) -> size_t {
      dislua::uleb128 vals[2];
      std::memcpy(vals, &arg, sizeof(arg));
      return 1 + uleb128_size(vals[0]) + uleb128_size(vals[1]);
    },
    [](const std::string &arg) -> size_t {
      size_t size = arg.size();
      return uleb128_size(lj::ktab::string + static_cast<dislua::uleb128>(size)) + size;
    }}, v);
}

size_t bclist_lj::table_size(dislua::table_t t) {
  size_t res = 0;
  dislua::uleb128 narray = 0, nhash = 0;

  dislua::leb128 i = 0;
  decltype(t)::iterator it;
  while (it = t.find(i++), it != t.end()) {
    res += table_kv_size(it->second);
    narray++;
    t.erase(it->first);
  }

  for (const auto &kv: t) {
    res += table_kv_size(kv.first) + table_kv_size(kv.second);
    nhash++;
  }

  res += uleb128_size(narray) + uleb128_size(nhash);
  return res;
}

bool bclist_lj::is_debug() const {
  return (info->header.flags & lj::dump_flags::strip) == 0;
}

dislua::uchar bclist_lj::bcmax() const {
  if (info->version == 1)
    return lj::v1::bcops::BCMAX;
  return lj::v2::bcops::BCMAX;
}

auto bclist_lj::bcopcode(size_t i) const {
  const auto opcodes = info->version == 1 ? lj::v1::opcodes : lj::v2::opcodes;
  return opcodes[i];
}

int bclist_lj::get_mode(dislua::uchar opcode) const {
  // rshift 0 = field a
  // rshift 3 = field b
  // rshift 7 = field c/d
  if (opcode >= bcmax())
    return lj::bcmode::none;
  return bcopcode(opcode).second;
}

std::string bclist_lj::header_flags() const {
  std::string res;
  dislua::uleb128 flags = info->header.flags;

  add_flag(res, flags, lj::dump_flags::be, "BCDUMP_F_BE");
  add_flag(res, flags, lj::dump_flags::strip, "BCDUMP_F_STRIP");
  add_flag(res, flags, lj::dump_flags::ffi, "BCDUMP_F_FFI");
  if (info->version == 2)
    add_flag(res, flags, lj::dump_flags::fr2, "BCDUMP_F_FR2");
  if (flags != 0)
    add_flag(res, flags);

  if (!res.empty())
    res.erase(res.size() - 3);

  return res;
}

std::string bclist_lj::fix_string(std::string_view str) const {
  std::string res = "\"";
  size_t newline = 0;

  for (const char &c: str) {
    if (is_newline(res.size() - newline)) {
      res += "\"\n.. \"";
      newline = res.size();
    }

    if (c == '\a')
      res.append("\\a");
    else if (c == '\b')
      res.append("\\b");
    else if (c == '\f')
      res.append("\\f");
    else if (c == '\n')
      res.append("\\n");
    else if (c == '\r')
      res.append("\\r");
    else if (c == '\t')
      res.append("\\t");
    else if (c == '\v')
      res.append("\\v");
    else if (c == '"')
      res.append("\\\"");
    else if (c == '\\')
      res.append("\\\\");
    else if (c < ' ' || c > '~')
      fmt::format_to(std::back_inserter(res), "\\x{:02x}", static_cast<unsigned char>(c));
    else
      res.push_back(c);
  }
  return res + "\"";
}

std::string bclist_lj::varname(const dislua::varname &vn) const {
  switch (vn.type) {
  case lj::varnames::index:
    return "(index)";
  case lj::varnames::limit:
    return "(limit)";
  case lj::varnames::step:
    return "(step)";
  case lj::varnames::generator:
    return "(generator)";
  case lj::varnames::state:
    return "(state)";
  case lj::varnames::control:
    return "(control)";
  default:
    return fix_string(vn.name);
  }
}

std::string bclist_lj::table_kv(const dislua::table_val_t &v) const {
  return std::visit(dislua::detail::overloaded{
    [](std::nullptr_t)             -> std::string { return "nil"; },
    [](bool arg)                   -> std::string { return arg ? "true" : "false"; },
    [](dislua::leb128 arg)         -> std::string { return std::to_string(arg); },
    [](double arg)                 -> std::string { return std::to_string(arg); },
    [this](const std::string &arg) -> std::string { return fix_string(arg); }
    }, v);
}

std::string bclist_lj::table(dislua::table_t t) {
  std::string res;
  size_t newline = 0;

  dislua::leb128 i = 1;
  decltype(t)::iterator it;
  while (it = t.find(i++), it != t.end()) {
    if (is_newline(res.size() - newline)) {
      res += "\n";
      newline = res.size();
    }
    res += table_kv(it->second) + ", ";
    t.erase(it->first);
  }

  for (const auto &kv: t) {
    if (kv.second.index() == 0) // std::nullptr_t => nil
      continue;

    if (is_newline(res.size() - newline)) {
      res += "\n";
      newline = res.size();
    }
    fmt::format_to(std::back_inserter(res), "[{}] = {}, ", table_kv(kv.first), table_kv(kv.second));
  }

  if (!res.empty())
    res.erase(res.size() - 2);

  return "{" + res + "}";
}

size_t bcproto_lj::kgc_size(const dislua::kgc_t &v) {
  return 1 + std::visit(dislua::detail::overloaded{
    [](const dislua::proto_id &) -> size_t { return 0; },
    [](const dislua::table_t &t) -> size_t { return table_size(t); },
    [](long long v)              -> size_t { return uleb128_sizes(v); },
    [](unsigned long long v)     -> size_t { return uleb128_sizes(v); },
    [](std::complex<double> v)   -> size_t { return uleb128_sizes(v.imag()) + uleb128_sizes(v.real()); },
    [](const std::string &v)     -> size_t {
      const size_t size = v.size();
      return uleb128_size(lj::kgc::string + static_cast<dislua::uleb128>(size)) - 1 + size;
    },
  }, v);
}

size_t bcproto_lj::knum_size(double val) {
  double sval = static_cast<double>(static_cast<int>(val)); // signed value
  if (dislua::detail::almost_equal(val, sval, 2))
    return uleb128_33_size(static_cast<dislua::uleb128>(val));
  else {
    dislua::uleb128 v[2] = {0};
    std::memcpy(v, &val, sizeof(val));
    return uleb128_33_size(v[0]) + uleb128_size(v[1]);
  }
}

size_t bcproto_lj::header_size() const {
  const dislua::uleb128 sizekgc = static_cast<dislua::uleb128>(ref().kgc.size()),
                        sizekn = static_cast<dislua::uleb128>(ref().knum.size()),
                        sizebc = static_cast<dislua::uleb128>(ref().ins.size());
  size_t res = sizeof(dislua::uchar) * 4 + uleb128_size(sizekgc) + uleb128_size(sizekn) + uleb128_size(sizebc);

  if (is_debug()) {
    size_t debug_size = lineinfo_size() + uvname_size() + varname_size();
    res += uleb128_size(static_cast<dislua::uleb128>(debug_size));
    if (debug_size)
      res += uleb128_size(ref().firstline) + uleb128_size(ref().numline);
  }
  return res;
}

size_t bcproto_lj::ins_size() const {
  return ref().ins.size() * sizeof(dislua::uint);
}

size_t bcproto_lj::uv_size() const {
  return ref().uv.size() * sizeof(dislua::ushort);
}

size_t bcproto_lj::kgc_size() const {
  return std::accumulate(ref().kgc.begin(), ref().kgc.end(), size_t{0}, [](size_t res, const dislua::kgc_t &v) {
    return res + kgc_size(v);
  });
}

size_t bcproto_lj::knum_size() const {
  return std::accumulate(ref().knum.begin(), ref().knum.end(), size_t{0}, [](size_t res, double v) {
    return res + knum_size(v);
  });
}

size_t bcproto_lj::lineinfo_size() const {
  const size_t size = ref().lineinfo.size();
  size_t res;
  if (ref().numline >= 1 << 16)
    res = size * 4;
  else if (ref().numline >= 1 << 8)
    res = size * 2;
  else
    res = size;
  return res;
}

size_t bcproto_lj::uvname_size() const {
  size_t res = 0;
  for (const std::string &name: ref().uv_names)
    res += name.size() + 1;
  return res;
}

size_t bcproto_lj::varname_size() const {
  size_t last = 0, res = 0;
  for (const dislua::varname &i: ref().varnames) {
    res++;
    if (i.type >= lj::varnames::MAX)
      res += i.name.size();

    res += uleb128_size(static_cast<dislua::uleb128>(i.start - last));
    last = i.start;
    res += uleb128_size(static_cast<dislua::uleb128>(i.end - i.start));
  }
  res++;

  return res;
}

std::string bcproto_lj::flags() const {
  std::string res;
  dislua::uchar flags = ref().flags;

  add_flag(res, flags, lj::proto_flags::child, "PROTO_CHILD");
  add_flag(res, flags, lj::proto_flags::varargs, "PROTO_VARARG");
  add_flag(res, flags, lj::proto_flags::ffi, "PROTO_FFI");
  add_flag(res, flags, lj::proto_flags::nojit, "PROTO_NOJIT");
  add_flag(res, flags, lj::proto_flags::iloop, "PROTO_ILOOP");
  if (flags != 0)
    add_flag(res, flags);

  if (!res.empty())
    res.erase(res.size() - 3);
  return res;
}

std::string bcproto_lj::fill_field(size_t i, int nfield) const {
  std::string res;
  const auto ins = ref().ins[i];
  const int mode = get_mode(ins.opcode);
  int m = 0, field = 0;

  switch (nfield) {
  case 0: // a
    m = mode & 7;
    field = ins.a;
    break;
  case 1: // b
    m = (mode >> 3) & lj::bcmode::MAX;
    field = ins.b;
    break;
  case 2: // c
    m = (mode >> 7) & lj::bcmode::MAX;
    field = ins.c;
    break;
  case 3: // d
    m = (mode >> 7) & lj::bcmode::MAX;
    field = ins.d;
    break;
  default:
    break;
  }

  size_t ufield = static_cast<size_t>(field);
  switch (m) {
  case lj::bcmode::uv:
    res = get_uv(ufield);
    break;
  case lj::bcmode::pri:
    res = get_pri(ufield);
    break;
  case lj::bcmode::num:
    res = get_knum(ufield);
    break;
  case lj::bcmode::str:
    res = get_kgc(ref().kgc.size() - 1 - ufield, lj::kgc::string);
    break;
  case lj::bcmode::tab:
    res = get_kgc(ref().kgc.size() - 1 - ufield, lj::kgc::tab);
    break;
  case lj::bcmode::func:
    res = get_kgc(ref().kgc.size() - 1 - ufield, lj::kgc::child);
    break;
  case lj::bcmode::jump:
    res = get_label(ufield + i + 1 - 0x8000);
    field -= 0x8000;
    break;
  // case lj::bcmode::cdata:
  //   get_kgc(ref().kgc.size() - 1 - field, ...);
  //   break;
  default:
    break;
  }

  std::string str_num = std::to_string(field);
  if (res.empty())
    return str_num;
  return res + " (" + str_num + ")";
}

bclist::div bcproto_lj::ins() {
  div res;
  if (ref().ins.empty())
    return res;
  res.header = ".ins";

  const size_t start = offset;
  const auto to_offset = [start](size_t i) {
    return start + i * sizeof(dislua::uint);
  };

  std::vector<size_t> jumps;
  for (size_t i = 0; i < ref().ins.size(); i++) {
    const auto ins = ref().ins[i];
    if (is_jump(get_mode(ins.opcode)))
      jumps.emplace_back(static_cast<size_t>(ins.d) + 1 + i - 0x8000);
  }

  size_t prev_line = 0;
  for (size_t i = 0; i < ref().ins.size(); i++) {
    const auto ins = ref().ins[i];
    if (std::find(jumps.begin(), jumps.end(), i) != jumps.end()) { // is a label
      if (!res.lines.empty())
        res.empty_line(to_offset(i) - sizeof(dislua::uint));
      new_line(res, 0, "{:s}:", get_label(i));
    }

    const std::string opcn = ins.opcode > bcmax() ? "UNK" : bcopcode(ins.opcode).first;
    std::string fields, comment;
    if (is_debug() && ref().lineinfo[i] != prev_line) {
      prev_line = ref().lineinfo[i];
      comment = fmt::format(" -- Line in source code: {:d}", prev_line);
    }

    const std::string field_a = fill_field(i, 0);
    fields.append(field_a + ",");

    if (has_b_field(get_mode(ins.opcode))) {
      const std::string field_b = fill_field(i, 1);
      const std::string field_c = fill_field(i, 2);

      fields.append(field_b + ",");
      fields.append(field_c);
    } else {
      const std::string field_d = fill_field(i, 3);
      fields.append(field_d);
    }

    new_line(res, sizeof(dislua::uint), "({:02X} {:02X} {:02X} {:02X}) {:s}\t{:s}{:s}", ins.opcode, ins.a, ins.c, ins.b,
             opcn, fields, comment);
  }
  res.empty_line();

  return res;
}

bclist::div bcproto_lj::uvdata() {
  div res;
  if (ref().uv.empty())
    return res;
  res.header = ".uvdata";

  for (size_t i = 0; i < ref().uv.size(); i++) {
    const dislua::ushort uv = ref().uv[i];
    new_line(res, sizeof(dislua::ushort), "{} = 0x{:04X}", get_uv(i), uv);
  }
  res.empty_line();

  return res;
}

bclist::div bcproto_lj::kgc() {
  div res;
  if (ref().kgc.empty())
    return res;
  res.header = ".kgc";

  for (size_t i = 0; i < ref().kgc.size(); i++) {
    const dislua::kgc_t kgc = ref().kgc[i];
    const dislua::uleb128 index = static_cast<dislua::uleb128>(kgc.index());
    const std::string val = std::visit(dislua::detail::overloaded{
      [&](const dislua::proto_id &id) -> std::string { return "proto" + std::to_string(id.id); },
      [&](const dislua::table_t &t)   -> std::string { return table(t); },
      [](long long v)                 -> std::string { return std::to_string(v); },
      [](unsigned long long v)        -> std::string { return std::to_string(v); },
      [](std::complex<double> v)      -> std::string { return fmt::format("({}+{}i)", v.real(), v.imag()); },
      [&](const std::string &str)     -> std::string { return fix_string(str); }
    }, kgc);

    new_line(res, kgc_size(kgc), "{} = {}", get_kgc(i, index), val);
  }
  res.empty_line();

  return res;
}

bclist::div bcproto_lj::knum() {
  div res;
  if (ref().knum.empty())
    return res;
  res.header = ".knum";

  for (size_t i = 0; i < ref().knum.size(); i++) {
    const double num = ref().knum[i], snum = static_cast<double>(static_cast<int>(num)); // signed value
    size_t size;
    if (dislua::detail::almost_equal(num, snum, 2))
      size = uleb128_33_size(static_cast<dislua::uleb128>(num));
    else {
      dislua::uleb128 v[2] = {0};
      std::memcpy(v, &num, sizeof(num));
      size = uleb128_33_size(v[0]) + uleb128_size(v[1]);
    }

    new_line(res, size, "{} = {}", get_knum(i), num);
  }
  res.empty_line();

  return res;
}

bclist::div bcproto_lj::operator()() {
  div res = {
    .tab = 1,
    .header = fmt::format("proto{} do", proto_id),
    .footer = "end\n"
  };

  div pinfo = { .header = ".info" };

  dislua::uleb128 debug_size = 0;
  if (is_debug())
    debug_size += static_cast<dislua::uleb128>(lineinfo_size() + uvname_size() + varname_size());

  // prototype size
  dislua::uleb128 proto_size = static_cast<dislua::uleb128>(header_size() + ins_size() + uv_size() + kgc_size() + knum_size()) + debug_size;
  new_line(pinfo, uleb128_size(proto_size), "size = {:08X}", proto_size);
  if (dislua::uchar fl = ref().flags)
    new_line(pinfo, sizeof(dislua::uchar), "flags = 0b{} -- {}", std::bitset<8>(fl).to_string(), flags());
  else
    new_line(pinfo, sizeof(dislua::uchar), "flags = 0");

  dislua::uleb128 sizekgc = static_cast<dislua::uleb128>(ref().kgc.size()),
                  sizekn = static_cast<dislua::uleb128>(ref().knum.size()),
                  sizebc = static_cast<dislua::uleb128>(ref().ins.size());
  new_line(pinfo, sizeof(dislua::uchar), "numparams = {:d}", ref().numparams);
  new_line(pinfo, sizeof(dislua::uchar), "framesize = {:d}", ref().framesize);
  new_line(pinfo, sizeof(dislua::uchar), "sizeuv = {:d}", ref().uv.size());
  new_line(pinfo, uleb128_size(sizekgc), "sizekgc = {:d}", sizekgc);
  new_line(pinfo, uleb128_size(sizekn), "sizekn = {:d}", sizekn);
  new_line(pinfo, uleb128_size(sizebc), "sizebc = {:d}", sizebc);

  if (is_debug()) {
    new_line(pinfo, uleb128_size(debug_size), "sizedbg = {:d}", debug_size);
    new_line(pinfo, uleb128_size(ref().firstline), "firstline = {:d}", ref().firstline);
    new_line(pinfo, uleb128_size(ref().numline), "numline = {:d}", ref().numline);
  }
  pinfo.empty_line();

  res.add_div(pinfo);
  res.add_div(ins());
  res.add_div(uvdata());
  res.add_div(kgc());
  res.add_div(knum());
  // res.add_div(varnames());

  // remove last empty line
  res.additional.back().lines.pop_back();

  offset += debug_size;
  return res;
}

// 0: compiler info
// 1: header info
// 2..: prototypes
void bclist_lj::update() {
  offset = 0;

  div compiler;
  compiler.empty_line(offset);
  new_line(compiler, 3, "-- Compiler: LuaJIT");
  new_line(compiler, 1, "-- Version: {}", info->version);
  compiler.empty_line();
  divs.add_div(compiler);

  div header;
  header.header = ".header";

  if (dislua::uint flags = info->header.flags) {
    new_line(header, uleb128_size(flags), "flags = 0b{} -- {}", std::bitset<8>(flags).to_string(), header_flags());
  } else {
    new_line(header, 1, "flags = 0");
  }

  if (is_debug()) {
    size_t s = info->header.debug_name.size();
    new_line(header, uleb128_size(static_cast<dislua::uleb128>(s)) + s, "debug_name = \"{}\"", info->header.debug_name);
  }

  header.empty_line();
  divs.add_div(header);

  for (size_t i = 0; i < info->protos.size(); ++i) {
    // FIXME
    bcproto_lj p{*this, i};

    divs.add_div(p());
    offset = p.offset;
    temp_protos_id = p.temp_protos_id;
    temp_protos_id.emplace_back(i);
  }
}
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

#include <bitset>
#include <iostream>

#include <fmt/core.h>

#include "bclist.hpp"

std::string tostring_header_flags(dislua::dump_info *info) {
  std::string res;
  dislua::uleb128 flags = info->header.flags;

  if (flags & dislua::lj::DUMP_BE)
    res += "DUMP_BE | ";
  if (flags & dislua::lj::DUMP_STRIP)
    res += "DUMP_STRIP | ";
  if (flags & dislua::lj::DUMP_FFI)
    res += "DUMP_FFI | ";
  if (info->version == 2 && flags & dislua::lj::DUMP_FR2)
    res += "DUMP_FR2 | ";

  if (!res.empty())
    res.erase(res.size() - 3);

  return res;
}

std::string tostring_proto_flags(dislua::dump_info::proto &p) {
  std::string res;
  dislua::uleb128 flags = p.flags;

  if (flags & dislua::lj::PROTO_CHILD)
    res += "PROTO_CHILD | ";
  if (flags & dislua::lj::PROTO_VARARG)
    res += "PROTO_VARARG | ";
  if (flags & dislua::lj::PROTO_FFI)
    res += "PROTO_FFI | ";
  if (flags & dislua::lj::PROTO_NOJIT)
    res += "PROTO_NOJIT | ";
  if (flags & dislua::lj::PROTO_ILOOP)
    res += "PROTO_ILOOP | ";

  if (!res.empty())
    res.erase(res.size() - 3);

  return res;
}

std::string fix_string(std::string &str) {
  std::string res = "\"";
  size_t newline = 0;
  
  for (char &c: str) {
    if (res.size() - newline > 50) {
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
      fmt::format_to(std::back_inserter(res), "\\x{:02x}",
                     static_cast<unsigned char>(c));
    else
      res.push_back(c);
  }
  return res + "\"";
}

std::string tostring_varname(dislua::dump_info::varname &vn) {
  namespace lj = dislua::lj;
  switch (vn.type) {
  case lj::VARNAME_FOR_IDX:
    return "(index)";
  case lj::VARNAME_FOR_STOP:
    return "(limit)";
  case lj::VARNAME_FOR_STEP:
    return "(step)";
  case lj::VARNAME_FOR_GEN:
    return "(generator)";
  case lj::VARNAME_FOR_STATE:
    return "(state)";
  case lj::VARNAME_FOR_CTL:
    return "(control)";
  default:
    return fix_string(vn.name);
  }
}

std::string table_kv_tostring(const dislua::dump_info::table_val_t &v) {
  std::string res;
  std::visit(dislua::overloaded{
                 [&](std::nullptr_t) { res = "nil"; },
                 [&](bool arg) { res = arg ? "true" : "false"; },
                 [&](dislua::leb128 arg) { res = std::to_string(arg); },
                 [&](double arg) { res = std::to_string(arg); },
                 [&](std::string arg) { res = fix_string(arg); }},
             v);
  return res;
}

std::string table_tostring(dislua::dump_info::table_t &t) {
  std::string res;
  dislua::dump_info::table_t copy_table = t;
  size_t newline = 0;

  dislua::leb128 i = 1;
  decltype(copy_table)::iterator it;
  while (it = copy_table.find(i++), it != copy_table.end()) {
    if (res.size() - newline > 50) {
      res += "\n";
      newline = res.size();
    }
    res += table_kv_tostring(it->second) + ", ";
    copy_table.erase(it->first);
  }

  for (auto &kv: copy_table) {
    if (kv.second.index() == 0)
      continue;
    
    if (res.size() - newline > 50) {
      res += "\n";
      newline = res.size();
    }
    fmt::format_to(std::back_inserter(res), "[{}] = {}, ",
                   table_kv_tostring(kv.first), table_kv_tostring(kv.second));
  }

  if (!res.empty())
    res.erase(res.size() - 2);

  return "{" + res + "}";
}

std::unique_ptr<bclist::div> lj_proto(size_t proto_id, dislua::dump_info *info,
                                      std::vector<size_t> &idxs) {
  namespace lj = dislua::lj;

  dislua::uint version = info->version;
  bool is_debug = (info->header.flags & dislua::lj::DUMP_STRIP) == 0;
  dislua::dump_info::proto &p = info->protos[proto_id];
  auto opcodes = version == 1 ? lj::v1::opcodes : lj::v2::opcodes;
  dislua::uchar max = version == 1 ? dislua::uchar(lj::v1::BC__MAX)
                                   : dislua::uchar(lj::v2::BC__MAX);

  auto get_mode = [&](dislua::uchar opcode, int shift = 0) -> int {
    // shift 0 = field a
    // shift 3 = field b
    // shift 7 = field c/d

    if (opcode >= max)
      return int(lj::BCM___);
    int mode = opcodes[opcode].second;
    if (shift)
      mode >>= shift;
    return (mode & lj::BCM_max);
  };
  auto get_uv = [&](size_t i) -> std::string {
    if (i >= p.uv.size())
      return "(error)";

    if (is_debug)
      return "uv_" + p.uv_names[i];
    std::string res;
    fmt::format_to(std::back_inserter(res), "uv_{:d}_{:d}", proto_id, i);
    return res;
  };
  auto get_label = [&](size_t i) -> std::string {
    if (i >= p.ins.size())
      return "(error)";

    std::string res;
    fmt::format_to(std::back_inserter(res), "label_{:d}_{:d}", proto_id, i);
    return res;
  };
  auto get_knum = [&](size_t i) -> std::string {
    if (i >= p.knum.size())
      return "(error)";

    std::string res;
    fmt::format_to(std::back_inserter(res), "knum_{:d}_{:d}", proto_id, i);
    return res;
  };
  auto get_kgc = [&](size_t i, lj::kgc mode) -> std::string {
    if (i >= p.kgc.size())
      return "(error)";

    bool is_correct_mode = true;
    std::visit(
        dislua::overloaded{
            [&](dislua::dump_info::proto) {
              is_correct_mode = mode == lj::KGC_CHILD;
            },
            [&](dislua::dump_info::table_t) {
              is_correct_mode = mode == lj::KGC_TAB;
            },
            [&](long long) { is_correct_mode = mode == lj::KGC_I64; },
            [&](unsigned long long) { is_correct_mode = mode == lj::KGC_U64; },
            [&](std::complex<double>) {
              is_correct_mode = mode == lj::KGC_COMPLEX;
            },
            [&](std::string) { is_correct_mode = mode == lj::KGC_STR; }},
        p.kgc[i]);

    if (!is_correct_mode)
      return "(error)";

    std::string res;
    fmt::format_to(std::back_inserter(res), "kgc_{:d}_{:d}", proto_id, i);
    return res;
  };

  auto is_jump = [&](dislua::uchar opcode) -> bool {
    return get_mode(opcode, 7) == lj::BCMjump;
  };
  auto has_b_field = [&](dislua::uchar opcode) -> bool {
    return get_mode(opcode, 3) != lj::BCM___;
  };
  auto is_uv = [&](dislua::uchar opcode, int shift = 0) -> bool {
    return get_mode(opcode) <= lj::BCMlits &&
           get_mode(opcode, shift) == lj::BCMuv;
  };
  auto is_num = [&](dislua::uchar opcode, int shift = 0) -> bool {
    return get_mode(opcode, shift) == lj::BCMnum;
  };
  auto is_str = [&](dislua::uchar opcode, int shift = 0) -> bool {
    return get_mode(opcode, shift) == lj::BCMstr;
  };
  auto is_func = [&](dislua::uchar opcode, int shift = 0) -> bool {
    return get_mode(opcode, shift) == lj::BCMfunc;
  };
  auto is_tab = [&](dislua::uchar opcode, int shift = 0) -> bool {
    return get_mode(opcode, shift) == lj::BCMtab;
  };

  auto res = std::make_unique<bclist::div>();
  fmt::format_to(std::back_inserter(res->header), "proto{} do\n", proto_id);

  std::vector<dislua::uint> jumps;
  dislua::uint i = 0;
  for (auto ins: p.ins) {
    if (is_jump(ins.opcode))
      jumps.push_back(static_cast<dislua::uint>(ins.d) + 1 + i - 0x8000);
    i++;
  }

  // 0: info
  // 1: ins
  // 2: uvdata
  // 3: kgc
  // 4: knum
  // 5: varnames (debug)

  // prototype info
  bclist::div pinfo;
  pinfo.header = "\t.info\n";
  if (p.flags != 0)
    fmt::format_to(std::back_inserter(pinfo.lines), "\tflags = 0b{} -- {}\n",
                   std::bitset<8>(p.flags).to_string(),
                   tostring_proto_flags(p));
  else
    pinfo.lines += "\tflags = 0b0\n";
  fmt::format_to(std::back_inserter(pinfo.lines),
                 "\tnumparams = {:d}\n"
                 "\tframesize = {:d}\n"
                 "\tsizeuv = {:d}\n"
                 "\tsizekgc = {:d}\n"
                 "\tsizekn = {:d}\n"
                 "\tsizebc = {:d}\n",
                 p.numparams, p.framesize, p.uv.size(), p.kgc.size(),
                 p.knum.size(), p.ins.size());

  if (is_debug)
    fmt::format_to(std::back_inserter(pinfo.lines),
                   "\tfirstline = {:d}\n"
                   "\tnumline = {:d}\n",
                   p.firstline, p.numline);
  res->additional.push_back(pinfo);

  // instructinos & lineinfo (debug)
  bclist::div pins;
  size_t prev_line = 0;
  if (p.ins.size() > 0) {
    i = 0;
    pins.header = "\n\t.ins\n";
    for (auto ins: p.ins) {
      if (std::find(jumps.begin(), jumps.end(), i) != jumps.end()) // is a label
        fmt::format_to(std::back_inserter(pins.lines), "{}:\n", get_label(i));
      fmt::format_to(std::back_inserter(pins.lines),
                     "\t{:04d}: {:02x} {:02x} {:02x} {:02x} ", i, ins.opcode,
                     ins.a, ins.c, ins.b);

      std::string opcode_name =
          ins.opcode >= max ? "UNK" : opcodes[ins.opcode].first;
      pins.lines += opcode_name + "\t";

      std::string field_a, field_b, field_cd, comment;
      if (is_debug && p.lineinfo[i] != prev_line) {
        prev_line = p.lineinfo[i];
        fmt::format_to(std::back_inserter(comment),
                       " -- Line in source code: {:d}", prev_line);
      }

      if (is_uv(ins.opcode))
        field_a = get_uv(ins.a);

      if (!field_a.empty())
        fmt::format_to(std::back_inserter(field_cd), " ({:d})", ins.a);
      else
        field_a = std::to_string(static_cast<int>(ins.a));

      if (has_b_field(ins.opcode)) {
        field_b = std::to_string(static_cast<int>(ins.b));

        if (is_num(ins.opcode, 7))
          field_cd = get_knum(ins.c);
        else if (is_str(ins.opcode, 7))
          field_cd = get_kgc(p.kgc.size() - ins.c - 1, lj::KGC_STR);

        if (!field_cd.empty())
          fmt::format_to(std::back_inserter(field_cd), " ({:d})", ins.c);
        else
          field_cd = std::to_string(static_cast<int>(ins.c));
      } else {
        if (is_jump(ins.opcode))
          field_cd = get_label(ins.d + i + 1 - 0x8000);
        else if (is_uv(ins.opcode, 7))
          field_cd = get_uv(ins.d);
        else if (is_num(ins.opcode, 7))
          field_cd = get_knum(ins.d);
        else if (is_str(ins.opcode, 7))
          field_cd = get_kgc(p.kgc.size() - ins.d - 1, lj::KGC_STR);
        else if (is_func(ins.opcode, 7))
          field_cd = get_kgc(p.kgc.size() - ins.d - 1, lj::KGC_CHILD);
        else if (is_tab(ins.opcode, 7))
          field_cd = get_kgc(p.kgc.size() - ins.d - 1, lj::KGC_TAB);

        if (!field_cd.empty())
          fmt::format_to(std::back_inserter(field_cd), " ({:d})", ins.d);
        else
          field_cd = std::to_string(static_cast<unsigned int>(ins.d));
      }

      pins.lines += field_a;
      if (!field_b.empty())
        pins.lines += "," + field_b;
      pins.lines += "," + field_cd;
      if (!comment.empty())
        pins.lines += comment;

      pins.lines += "\n";
      i++;
    }
  }
  res->additional.push_back(pins);

  // upvalue data & names (debug)
  bclist::div puvdata;
  if (p.uv.size() > 0) {
    puvdata.header = "\n\t.uvdata\n";
    i = 0;
    for (auto uv: p.uv) {
      fmt::format_to(std::back_inserter(puvdata.lines), "\t{} = 0x{:04x}\n",
                     get_uv(i), uv);
      i++;
    }
  }
  res->additional.push_back(puvdata);

  // constant garbage collector objects
  bclist::div pkgc;
  if (p.kgc.size() > 0) {
    pkgc.header = "\n\t.kgc\n";
    i = 0;
    for (auto kgc: p.kgc) {
      std::visit(
          dislua::overloaded{
              [&](dislua::dump_info::proto) {
                fmt::format_to(std::back_inserter(pkgc.lines),
                               "\t{} = proto{}\n", get_kgc(i, lj::KGC_CHILD),
                               idxs.back());
                idxs.pop_back();
              },
              [&](dislua::dump_info::table_t arg) {
                fmt::format_to(std::back_inserter(pkgc.lines), "\t{} = {}\n",
                               get_kgc(i, lj::KGC_TAB), table_tostring(arg));
              },
              [&](long long arg) {
                fmt::format_to(std::back_inserter(pkgc.lines), "\t{} = {}\n",
                               get_kgc(i, lj::KGC_I64), arg);
              },
              [&](unsigned long long arg) {
                fmt::format_to(std::back_inserter(pkgc.lines), "\t{} = {}\n",
                               get_kgc(i, lj::KGC_U64), arg);
              },
              [&](std::complex<double> arg) {
                fmt::format_to(std::back_inserter(pkgc.lines),
                               "\t{} = {}+{}i\n", get_kgc(i, lj::KGC_COMPLEX),
                               arg.real(), arg.imag());
              },
              [&](std::string arg) {
                fmt::format_to(std::back_inserter(pkgc.lines),
                               "\t{} = {}\n", get_kgc(i, lj::KGC_STR),
                               fix_string(arg));
              }},
          kgc);
      i++;
    }
  }
  res->additional.push_back(pkgc);

  
  // constant numbers
  bclist::div pknum;
  if (p.knum.size() > 0) {
    pknum.header = "\n\t.knum\n";
    i = 0;
    for (auto num: p.knum) {
      fmt::format_to(std::back_inserter(pknum.lines), "\t{} = {}\n",
                     get_knum(i), num);
      i++;
    }
  }
  res->additional.push_back(pknum);

  // variable names
  bclist::div pvarnames;
  if (p.varnames.size() > 0) {
    pvarnames.header = "\n\t.varnames\n";
    i = 0;
    for (auto varname: p.varnames) {
      fmt::format_to(std::back_inserter(pvarnames.lines),
                     "\t<{:d}:{:d}> = {}\n", varname.start, varname.end,
                     tostring_varname(varname));
      i++;
    }
  }
  res->additional.push_back(pvarnames);

  res->footer = "end\n\n";
  return res;
}

// 0: compiler info
// 1: header info
// 2..: prototypes
void bclist::lj_update() {
  div compiler;
  fmt::format_to(std::back_inserter(compiler.lines),
                 "-- Compiler: LuaJIT\n"
                 "-- Version: {}\n",
                 info->version);
  compiler.footer = "\n";
  divs.push_back(compiler);

  div header;
  header.header = ".header\n";
  header.footer = "\n";
  if (info->header.flags != 0)
    fmt::format_to(std::back_inserter(header.lines), "flags = 0b{} -- {}\n",
                   std::bitset<8>(info->header.flags).to_string(),
                   tostring_header_flags(info));
  else
    header.lines += "flags = 0b0\n";

  bool is_debug = (info->header.flags & dislua::lj::DUMP_STRIP) == 0;
  if (is_debug)
    fmt::format_to(std::back_inserter(header.lines), "debug_name = \"{}\"\n",
                   info->header.debug_name);
  divs.push_back(header);

  std::vector<size_t> idxs;
  for (size_t i = 0; i < info->protos.size(); ++i) {
    std::unique_ptr<div> p = lj_proto(i, info, idxs);
    divs.push_back(*p.get());
    idxs.push_back(i);
  }
}

std::string bclist::lj_full() {
  std::string res;

  // compiler info
  auto ci = divs[0];
  res += ci.lines + ci.footer;

  // header info
  auto hi = divs[1];
  res += hi.header + hi.lines + hi.footer;

  // prototypes
  for (size_t i = 2; i < divs.size(); ++i) {
    auto pi = divs[i];
    res += pi.header;

    for (auto add: pi.additional) {
      if (add.header.empty())
        continue;
      res += add.header + add.lines + add.footer;
    }

    res += pi.footer;
  }

  return res;
}
// Luad - Disassembler for compiled Lua scripts.
// https://github.com/imring/Luad
// Copyright (C) 2021-2023 Vitaliy Vorobets
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

#include "customfuncs.hpp"

#include "../file.hpp"
#include "../mainwindow.hpp"

std::string tostring(sol::state_view lua, const sol::object &o) {
    return lua["tostring"](o);
}

void LuaCustom::initialize(LuaPlugin &plugin) {
    auto &lua = plugin.state;
    lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::bit32);

    // enums
    lua.new_enum("compilers", 
        "unknown", dislua::compilers::unknown, 
        "luajit", dislua::compilers::luajit
    );

    sol::table luajit_table = lua.create_table();
    lua.set("luajit", luajit_table);
    luajit_table.set("dump_flags", lua.create_table_with(
        "be", dislua::lj::dump_flags::be,
        "strip", dislua::lj::dump_flags::strip,
        "ffi", dislua::lj::dump_flags::ffi,
        "fr2", dislua::lj::dump_flags::fr2
    ));
    luajit_table.set("proto_flags", lua.create_table_with(
        "child", dislua::lj::proto_flags::child,
        "varargs", dislua::lj::proto_flags::varargs,
        "ffi", dislua::lj::proto_flags::ffi,
        "nojit", dislua::lj::proto_flags::nojit,
        "iloop", dislua::lj::proto_flags::iloop
    ));
    luajit_table.set("kgc", lua.create_table_with(
        "child", dislua::lj::kgc::child,
        "tab", dislua::lj::kgc::tab,
        "i64", dislua::lj::kgc::i64,
        "u64", dislua::lj::kgc::u64,
        "complex", dislua::lj::kgc::complex,
        "string", dislua::lj::kgc::string
    ));
    luajit_table.set("ktab", lua.create_table_with(
        "nil", dislua::lj::ktab::nil,
        "fal", dislua::lj::ktab::fal,
        "tru", dislua::lj::ktab::tru,
        "integer", dislua::lj::ktab::integer,
        "number", dislua::lj::ktab::number,
        "string", dislua::lj::ktab::string
    ));
    luajit_table.set("varnames", lua.create_table_with(
        "finish", dislua::lj::varnames::end,
        "index", dislua::lj::varnames::index,
        "limit", dislua::lj::varnames::limit,
        "step", dislua::lj::varnames::step,
        "generator", dislua::lj::varnames::generator,
        "state", dislua::lj::varnames::state,
        "control", dislua::lj::varnames::control,
        "MAX", dislua::lj::varnames::MAX
    ));
    luajit_table.set("bcmode", lua.create_table_with(
        "none", dislua::lj::bcmode::none,
        "dst", dislua::lj::bcmode::dst,
        "base", dislua::lj::bcmode::base,
        "var", dislua::lj::bcmode::var,
        "rbase", dislua::lj::bcmode::rbase,
        "uv", dislua::lj::bcmode::uv,
        "lit", dislua::lj::bcmode::lit,
        "lits", dislua::lj::bcmode::lits,
        "pri", dislua::lj::bcmode::pri,
        "num", dislua::lj::bcmode::num,
        "str", dislua::lj::bcmode::str,
        "tab", dislua::lj::bcmode::tab,
        "func", dislua::lj::bcmode::func,
        "jump", dislua::lj::bcmode::jump,
        "cdata", dislua::lj::bcmode::cdata,
        "MAX", dislua::lj::bcmode::MAX
    ));

    sol::table luajitv1_opcodes_table = lua.create_table();
    for (int i = 0; i < std::size(dislua::lj::v1::opcodes); i++) {
        const auto &[name, mode] = dislua::lj::v1::opcodes[i];
        luajitv1_opcodes_table.set(i, lua.create_table_with(
            "name", name,
            "mode", mode
        ));
    }
    luajit_table.set("v1", lua.create_table_with("opcodes", luajitv1_opcodes_table));

    sol::table luajitv2_opcodes_table = lua.create_table();
    for (int i = 0; i < std::size(dislua::lj::v2::opcodes); i++) {
        const auto &[name, mode] = dislua::lj::v2::opcodes[i];
        luajitv2_opcodes_table.set(i, lua.create_table_with(
            "name", name,
            "mode", mode
        ));
    }
    luajit_table.set("v2", lua.create_table_with("opcodes", luajitv2_opcodes_table));

    // functions
    lua.set_function("print", [&plugin](const sol::variadic_args &args) { LuaCustom::print(plugin, args); });
    lua.set_function("highlight", &LuaCustom::highlight);

    // types (separate functions to avoid large obj files)
    initialize_dislua_types(lua);
    initialize_bclist_types(lua);
}

void LuaCustom::print(LuaPlugin &plugin, const sol::variadic_args &args) {
    std::string result;
    bool               first = true;
    for (const auto &arg: args) {
        if (!first) {
            result.append("\t");
        }
        result.append(tostring(args.lua_state(), arg));
        first = false;
    }
    plugin.message(result);
}

void LuaCustom::highlight(int from, int to, int color) {
    MainWindow::instance()->highlight(from, to, color);
}
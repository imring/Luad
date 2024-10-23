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

#include "dislua/dislua.hpp"

template<typename T>
sol::object convert_value(sol::state_view &lua, const T& value) {
    return std::visit([&lua](auto&& arg) -> sol::object {
        using U = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<U, std::nullptr_t>) {
            return sol::nil;
        }
        return sol::make_object(lua, arg);
    }, value);
}

sol::table convert_table(sol::state_view &lua, const dislua::table_t &table) {
    sol::table result = lua.create_table();
    for (const auto &[key, value]: table) {
        result[convert_value(lua, key)] = convert_value(lua, value);
    }
    return result;
}

// for lua
class kgc_variant {
public:
    template<typename T>
    kgc_variant(T&& t): variant_{std::forward<T>(t)} {}

    sol::object value(sol::state_view &lua) const {
        return std::visit(dislua::detail::overloaded{
            [&lua](const dislua::proto_id &id) -> sol::object { return sol::make_object(lua, id.id); },
            [&lua](const dislua::table_t &t)   -> sol::object { return convert_table(lua, t); },
            [&lua](long long v)                -> sol::object { return sol::make_object(lua, v); },
            [&lua](unsigned long long v)       -> sol::object { return sol::make_object(lua, v); },
            [&lua](std::complex<double> v)     -> sol::object { return sol::make_object(lua, v); },
            [&lua](const std::string &str)     -> sol::object { return sol::make_object(lua, str); }
        }, variant_);
    }

    std::string type() const {
        return std::visit(dislua::detail::overloaded{
            [](const dislua::proto_id &id) -> std::string { return "proto"; },
            [](const dislua::table_t &t)   -> std::string { return "table"; },
            [](long long v)                -> std::string { return "ll"; },
            [](unsigned long long v)       -> std::string { return "ull"; },
            [](std::complex<double> v)     -> std::string { return "complex"; },
            [](const std::string &str)     -> std::string { return "string"; }
        }, variant_);
    }

private:
    dislua::kgc_t variant_;
};

void LuaCustom::initialize_dislua_types(sol::state &lua) {
    lua.new_usertype<dislua::varname>("varname",
        sol::call_constructor, sol::no_constructor,
        "type", &dislua::varname::type,
        "name", &dislua::varname::name,
        "start", &dislua::varname::start,
        "end", &dislua::varname::end
    );

    lua.new_usertype<kgc_variant>("kgc",
        sol::call_constructor, sol::no_constructor,
        "value", [&lua](kgc_variant &v) { return v.value(lua); },
        "type", &kgc_variant::type
    );

    lua.new_usertype<dislua::instruction>("instruction",
        sol::call_constructor, sol::no_constructor,
        "opcode", &dislua::instruction::opcode,
        "a", &dislua::instruction::a,
        "b", &dislua::instruction::b,
        "c", &dislua::instruction::c,
        "d", &dislua::instruction::d
    );
    
    lua.new_usertype<dislua::proto>("proto",
        sol::call_constructor, sol::no_constructor,
        "flags", &dislua::proto::flags,
        "numparams", &dislua::proto::numparams,
        "framesize", &dislua::proto::framesize,
        "firstline", &dislua::proto::firstline,
        "numline", &dislua::proto::numline,
        "instructions", [](dislua::proto &p) { return sol::as_table(p.ins); },
        "uv_values", [](dislua::proto &p) { return sol::as_table(p.uv); },
        "kgc_values", [&lua](dislua::proto &p) {
            sol::table result = lua.create_table();
            for (int i = 0; i < p.kgc.size(); i++) {
                result[i + 1] = kgc_variant{p.kgc[i]};
            }
            return result;
        },
        "knum_values", [](dislua::proto &p) { return sol::as_table(p.knum); },
        "lineinfo", [](dislua::proto &p) { return sol::as_table(p.lineinfo); },
        "uv_names", [](dislua::proto &p) { return sol::as_table(p.uv_names); },
        "varnames", [](dislua::proto &p) { return sol::as_table(p.varnames); }
    );

    lua.new_usertype<dislua::dump_info>("dump_info",
        sol::call_constructor, sol::no_constructor,
        "header_flags", [](dislua::dump_info &i) { return i.header.flags; },
        "debug_name", [](dislua::dump_info &i) { return i.header.debug_name; },
        "version", &dislua::dump_info::version,
        "protos", [](dislua::dump_info &i) { return sol::as_table(i.protos); },
        "buffer", &dislua::dump_info::buf,

        "compiler", [](dislua::dump_info &i) { return i.compiler(); }
    );
}
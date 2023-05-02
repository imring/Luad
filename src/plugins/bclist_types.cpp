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

#include "bclist.hpp"
#include "../file.hpp"

void LuaCustom::initialize_bclist_types(sol::state &lua) {
    lua.new_usertype<bclist::div::line>("bclistline",
        sol::call_constructor, sol::no_constructor,
        "text", &bclist::div::line::text,
        "key", &bclist::div::line::key,
        "from", &bclist::div::line::from,
        "to", &bclist::div::line::to
    );

    lua.new_usertype<bclist::div>("bclistdiv",
        sol::call_constructor, sol::no_constructor,
        "key", &bclist::div::key,
        "header", &bclist::div::header,
        "footer", &bclist::div::footer,
        "lines", [](bclist::div &d) { return sol::as_table(d.lines); },
        "additional", [](bclist::div &d) { return sol::as_table(d.additional); },

        "string", &bclist::div::string,
        "only_lines", &bclist::div::only_lines,
        "empty", &bclist::div::empty,
        "start", &bclist::div::start,
        "ends", &bclist::div::end
    );

    lua.new_usertype<bclist>("bclist",
        sol::call_constructor, sol::no_constructor,
        "refs", [&lua](bclist &b) {
            sol::table result = lua.create_table();
            for (const auto &[key, value]: b.refs) {
                result[key] = sol::as_table(value);
            }
            return result;
        },
        "divs", &bclist::divs,
        "info", &bclist::info
    );

    lua.new_usertype<File>("File",
        sol::call_constructor, sol::no_constructor,
        "path", [](File &f) { return f.path.toStdString(); },
        "dump_info", [](File &f) { return f.dump_info.get(); },
        "is_opened", [](File &f) { return f.is_opened(); }
    );
}
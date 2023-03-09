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

#ifndef LUAD_FILE_HPP
#define LUAD_FILE_HPP

#include <QString>

#include "bclist.hpp"

struct File {
    QString                 path;
    std::unique_ptr<bclist> dump_info;

    File() = default;
    File(QString path);

    bool open(QString path);
    bool save();
    void close();

    bool is_opened() const {
        return !path.isEmpty() && dump_info;
    };
};

#endif // LUAD_FILE_HPP
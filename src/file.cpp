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

#include "file.hpp"

#include <QFile>
#include <QMessageBox>

File::File(QString path) {
    open(path);
}

bool File::open(QString p) {
    QFile f{p};
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(nullptr, "Warning", "Cannot open file: " + f.errorString());
        return false;
    }

    const QByteArray blob = f.readAll();
    dislua::buffer   buf(blob.begin(), blob.end());
    auto             info = dislua::read_all(buf);
    if (!info) {
        QMessageBox::warning(nullptr, "Warning", "Unknown compiler of Lua script.");
        return false;
    }

    path      = p;
    dump_info = bclist::get_list(*info);
    dump_info->update();
    return true;
}

bool File::save() {
    dump_info->info->write();
    QFile f{path};
    if (!f.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(nullptr, "Warning", "Cannot open file: " + f.errorString());
        return false;
    }

    const auto buf = dump_info->info->buf.copy_data();
    f.write(std::bit_cast<const char *>(buf.data()), buf.size());
    return true;
}

void File::close() {
    path = "";
    dump_info.reset();
}
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

#ifndef LUAD_SETTINGS_HPP
#define LUAD_SETTINGS_HPP

#include <QString>
#include <QVariant>
#include <QSettings>

class Settings {
public:
    void     setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = {}) const;

    static Settings *instance();

    static inline const QString windowSizeKey     = "window_size";
    static inline const QString windowPositionKey = "window_position";

private:
    Settings() = default;

    QSettings settings;
};

#endif // LUAD_SETTINGS_HPP
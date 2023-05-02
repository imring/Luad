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

#ifndef LUAD_LINEHIGHLIGHTER_HPP
#define LUAD_LINEHIGHLIGHTER_HPP

#include <QTextEdit>
#include <QTextCharFormat>
#include <QRegularExpression>

class LineHighlighter : public QObject {
    Q_OBJECT
public:
    LineHighlighter() = default;

    void add(QTextCursor cursor);
    void add(QTextCursor cursor, QColor col);

    QList<QTextEdit::ExtraSelection> list() const { return list_; }

signals:
    void onAdded();

private:
    QList<QTextEdit::ExtraSelection> list_;
};

#endif // LUAD_LINEHIGHLIGHTER_HPP
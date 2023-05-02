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

#include "linehighlighter.hpp"

#include <QColorDialog>

#include "disassembler.hpp"

void LineHighlighter::add(QTextCursor cursor) {
    const QColor col = QColorDialog::getColor();
    if (!col.isValid()) {
        return;
    }
    add(cursor, col);
}

void LineHighlighter::add(QTextCursor cursor, QColor col) {
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(col);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);

    QTextCursor start = cursor;
    start.setPosition(cursor.selectionStart());
    QTextCursor end = cursor;
    end.setPosition(cursor.selectionEnd());
    while (start <= end && !start.atEnd()) {
        // start.movePosition(QTextCursor::EndOfLine);
        selection.cursor = start;
        list_.append(selection);
        // start.movePosition(QTextCursor::StartOfLine);
        start.movePosition(QTextCursor::Down);
    }

    emit onAdded();
}
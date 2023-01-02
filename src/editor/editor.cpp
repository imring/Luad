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

#include "editor.hpp"

constexpr int tabsize = 4;

size_t line_by_addr(const std::vector<bclist::div::line> &lines, size_t addr) {
    const auto binary_search = [](const std::vector<bclist::div::line> &lines, size_t addr, size_t low, size_t high) {
        while (low <= high) {
            const size_t mid = low + (high - low) / 2;
            const auto  &l   = lines[mid];
            if (l.from <= addr && l.to >= addr)
                return mid;
            if (l.from < addr)
                low++;
            else
                high--;
        }
        return bclist::max_line;
    };

    size_t res = binary_search(lines, addr, 0, lines.size());
    if (res != bclist::max_line) {
        while (res != 0 && lines[res - 1].from <= addr && lines[res - 1].to >= addr)
            res--;
    }
    return res;
}

Editor::Editor(QWidget *parent) : QPlainTextEdit{parent}, lineNumberArea{new LineNumberArea(this)} {
    connect(this, &Editor::blockCountChanged, this, &Editor::updateLineNumberAreaWidth);
    connect(this, &Editor::updateRequest, this, &Editor::updateLineNumberArea);
    connect(this, &Editor::cursorPositionChanged, this, &Editor::highlightCurrentLine);

    highlighter = new Highlighter{document()};

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    setReadOnly(true);
    setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    setTabStopDistance(40);
    // setTabStopDistance(fontMetrics().horizontalAdvance(QLatin1Char(' ')) * tabsize);
}

int Editor::lineNumberAreaWidth() const {
    int space = 3;
    if (lines.empty())
        return space;

    int digits = 1;
    int max    = lines.back().from;
    while (max >= 16) {
        max /= 16;
        ++digits;
    }

    space += fontMetrics().horizontalAdvance(QLatin1Char('9')) * qMax(digits, 8);
    return space;
}

void Editor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block       = firstVisibleBlock();
    int        blockNumber = block.blockNumber();
    int        top         = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int        bottom      = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top() && lines.size() > blockNumber) {
            QString number = QStringLiteral("%1").arg(lines[blockNumber].from, 8, 16, QLatin1Char('0'));
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);
        }

        block  = block.next();
        top    = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void Editor::resizeEvent(QResizeEvent *e) {
    QPlainTextEdit::resizeEvent(e);

    const QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void Editor::updateLineNumberAreaWidth(int /* newBlockCount */) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void Editor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection        selection;

    const QColor lineColor = QColor(Qt::yellow).lighter(160);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);

    setExtraSelections(extraSelections);
}

void Editor::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void Editor::open(QString path) {
    QFile file{path};
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Warning", "Cannot open file: " + file.errorString());
        return;
    }

    const QByteArray     blob = file.readAll();
    const dislua::buffer buf(blob.begin(), blob.end());
    const auto           info = dislua::read_all(buf);
    if (!info) {
        QMessageBox::warning(this, "Warning", "Unknown compiler of Lua script.");
        return;
    }

    filepath = path;
    ptr      = bclist::get_list(*info);

    ptr->update();
    bclist::div &&only_lines = ptr->divs.only_lines();
    lines                    = only_lines.lines;
    setPlainText(QString::fromStdString(only_lines.string()));

    emit onOpenFile(path);
}

bool Editor::jump(int addr) {
    const size_t l = line_by_addr(lines, addr);
    if (l == bclist::max_line) {
        return false;
    }

    QTextCursor cursor{document()->findBlockByNumber(l)};
    setTextCursor(cursor);

    return true;
}
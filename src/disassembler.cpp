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

#include "disassembler.hpp"

#include <QMenu>
#include <QPainter>
#include <QClipboard>
#include <QTextBlock>
#include <QTextCursor>
#include <QGuiApplication>
#include <QTextDocumentFragment>

#include "utils.hpp"
#include "xrefmenu.hpp"

Disassembler::Disassembler(QWidget *parent, std::weak_ptr<File> file)
    : QPlainTextEdit{parent}, file{file}, lineNumberArea{new LineNumberArea{this}}, syntaxHighlighter{new SyntaxHighlighter{document()}},
      contextMenu{new QMenu{"Context menu", this}}, lineHighlighter{} {
    setContextMenuPolicy(Qt::CustomContextMenu);

    QFont fontText;
    fontText.setFamily("Courier New");
    fontText.setFixedPitch(true);
    fontText.setPointSize(10);
    setFont(fontText);

    if (auto ptr = file.lock()) {
        bclist::div &&only_lines = ptr->dump_info->divs.only_lines();
        lines                    = only_lines.lines;
        setPlainText(QString::fromStdString(only_lines.string()));

        for (auto &line: lines) {
            if (!line.key.empty()) {
                addrKeys.emplace(line.key, line.from);
            }
        }
    }

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    setReadOnly(true);
    setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    setLineWrapMode(QPlainTextEdit::NoWrap);

    setTabStopDistance(40);
    // setTabStopDistance(fontMetrics().horizontalAdvance(QLatin1Char(' ')) * tabsize);

    connect(this, &Disassembler::blockCountChanged, this, &Disassembler::updateLineNumberAreaWidth);
    connect(this, &Disassembler::updateRequest, this, &Disassembler::updateLineNumberArea);
    connect(this, &Disassembler::cursorPositionChanged, this, &Disassembler::highlightCurrentLine);
    connect(this, &Disassembler::customContextMenuRequested, this, &Disassembler::showContextMenu);
    connect(&lineHighlighter, &LineHighlighter::onAdded, this, &Disassembler::highlightCurrentLine);
}

int Disassembler::lineNumberAreaWidth() const {
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

void Disassembler::lineNumberAreaPaintEvent(QPaintEvent *event) {
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

void Disassembler::resizeEvent(QResizeEvent *e) {
    QPlainTextEdit::resizeEvent(e);

    const QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void Disassembler::updateLineNumberAreaWidth(int /* newBlockCount */) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void Disassembler::highlightCurrentLine() {
    QTextEdit::ExtraSelection selection;

    const QColor lineColor = QColor(Qt::yellow).lighter(160);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();

    auto list = lineHighlighter.list();
    list.append(selection);
    setExtraSelections(list);
}

void Disassembler::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

bool Disassembler::jump(std::size_t addr, bool last) {
    const size_t l = utils::line_by_addr(lines, addr, last);
    if (l == bclist::max_line) {
        return false;
    }

    QTextCursor cursor{document()->findBlockByNumber(l)};
    setTextCursor(cursor);
    return true;
}

bool Disassembler::jump(std::string_view name) {
    const auto it = addrKeys.find(name);
    if (it == addrKeys.end()) {
        return false;
    }

    size_t l = utils::line_by_addr(lines, it->second);
    if (l == bclist::max_line) {
        return false;
    }

    while (lines[l].key != name && lines[l].from <= it->second && lines[l].to >= it->second) {
        l++;
    }
    QTextCursor cursor{document()->findBlockByNumber(l)};
    setTextCursor(cursor);
    return true;
}

std::size_t Disassembler::getCurrentAddress() const {
    const int index = textCursor().block().blockNumber();
    if (index < 0 || index >= lines.size()) {
        return 0;
    }
    const auto currentLine = lines[index];
    return currentLine.from;
}

void Disassembler::showContextMenu(const QPoint &pos) {
    contextMenu->clear();
    auto ptr = file.lock();
    if (!ptr) {
        return;
    }

    const int index = textCursor().block().blockNumber();
    if (index < 0 || index >= lines.size()) {
        return;
    }
    const auto currentLine = lines[index];

    QAction *actionCopy = nullptr, *actionAddress = nullptr, *actionGotoDef = nullptr, *actionHighlight = nullptr;
    QAction *actionXref[2] = {nullptr, nullptr};
    if (!textCursor().selection().isEmpty()) {
        actionCopy = new QAction{"Copy", this};
        actionCopy->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_C});
        contextMenu->addAction(actionCopy);
    }

    actionHighlight = new QAction{"Highlight line(s)", this};
    contextMenu->addAction(actionHighlight);
    contextMenu->addSeparator();

    actionAddress = new QAction{"Copy an address", this};
    contextMenu->addAction(actionAddress);

    textCursor().select(QTextCursor::WordUnderCursor);
    const QString     word    = textCursor().selectedText();
    const std::string stdword = word.toStdString();
    textCursor().clearSelection();
    bool hasWord = !word.isEmpty() && stdword != currentLine.key && addrKeys.find(stdword) != addrKeys.end();

    if (hasWord) {
        actionGotoDef = new QAction{QStringLiteral("Go to definition of \"%1\"").arg(word), this};
        contextMenu->addAction(actionGotoDef);
    }
    if (!currentLine.key.empty()) {
        actionXref[0] = new QAction{QStringLiteral("Find xrefs for \"%1\"").arg(QString::fromStdString(currentLine.key)), this};
        contextMenu->addAction(actionXref[0]);
    }
    if (hasWord) {
        actionXref[1] = new QAction{QStringLiteral("Find xrefs for \"%1\"").arg(word), this};
        contextMenu->addAction(actionXref[1]);
    }

    const QAction *action = contextMenu->exec(mapToGlobal(pos));
    if (action) {
        if (action == actionCopy) {
            QGuiApplication::clipboard()->setText(textCursor().selectedText());
        } else if (action == actionAddress) {
            QString address = QStringLiteral("%1").arg(currentLine.from, 8, 16, QLatin1Char('0'));
            QGuiApplication::clipboard()->setText(address);
        } else if (action == actionXref[0] || action == actionXref[1]) {
            const std::string current = action == actionXref[0] ? currentLine.key : stdword;
            XrefMenu         *menu    = new XrefMenu{this, file, addrKeys.find(current)->second};
            emit              showXref(QStringLiteral("Xref for \"%1\"").arg(QString::fromStdString(current)), menu);
        } else if (action == actionGotoDef) {
            jump(stdword);
        } else if (action == actionHighlight) {
            lineHighlighter.add(textCursor());
        }
    }
}
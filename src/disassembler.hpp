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

#ifndef LUAD_DISASSEMBLER_HPP
#define LUAD_DISASSEMBLER_HPP

#include <QPlainTextEdit>

#include "file.hpp"
#include "linehighlighter.hpp"
#include "syntaxhighlighter.hpp"

class LineNumberArea;
class XrefMenu;

class Disassembler : public QPlainTextEdit {
    Q_OBJECT

public:
    Disassembler(QWidget *parent = nullptr, std::weak_ptr<File> file = {});

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int  lineNumberAreaWidth() const;

    bool jump(std::size_t addr, bool last = false);
    bool jump(std::string_view name);

protected:
    void resizeEvent(QResizeEvent *event) override;

signals:
    void showXref(const QString &name, XrefMenu *menu);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
    void showContextMenu(const QPoint &pos);

private:
    LineNumberArea    *lineNumberArea;
    SyntaxHighlighter *syntaxHighlighter;

    LineHighlighter lineHighlighter;

    QMenu *contextMenu;

    std::weak_ptr<File>                     file;
    std::vector<bclist::div::line>          lines;
    std::map<std::string_view, std::size_t> addrKeys;
};

class LineNumberArea : public QWidget {
public:
    LineNumberArea(Disassembler *disasm) : QWidget{disasm}, disasm{disasm} {}

    QSize sizeHint() const override {
        return QSize(disasm->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        disasm->lineNumberAreaPaintEvent(event);
    }

private:
    Disassembler *disasm;
};

#endif // LUAD_DISASSEMBLER_HPP
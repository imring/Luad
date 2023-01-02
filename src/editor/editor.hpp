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

#ifndef LUAD_EDITOR_HPP
#define LUAD_EDITOR_HPP

#include <QtWidgets>
#include <QPlainTextEdit>

#include "bclist.hpp"
#include "highlighter.hpp"

class LineNumberArea;

class Editor : public QPlainTextEdit {
    Q_OBJECT

public:
    Editor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int  lineNumberAreaWidth() const;

    void open(QString path);
    bool jump(int addr);

    const bclist *ptrinfo() const { return ptr.get(); }

protected:
    void resizeEvent(QResizeEvent *event) override;

signals:
    void onOpenFile(QString path = "");

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    LineNumberArea *lineNumberArea;
    Highlighter    *highlighter;

    QString                        filepath;
    std::unique_ptr<bclist>        ptr;
    std::vector<bclist::div::line> lines;
};

class LineNumberArea : public QWidget {
public:
    LineNumberArea(Editor *editor) : QWidget{editor}, editor{editor} {}

    QSize sizeHint() const override {
        return QSize(editor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        editor->lineNumberAreaPaintEvent(event);
    }

private:
    Editor *editor;
};

#endif // LUAD_EDITOR_HPP
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

#ifndef LUAD_MAINWINDOW_H
#define LUAD_MAINWINDOW_H

#include <QtWidgets>
#include <QMenuBar>

#include "editor/editor.hpp"
#include "editor/functions.hpp"
#include "settings.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    
    void closeEvent(QCloseEvent *event) override;

public slots:
    void open();
    void jumpTo();

private:
    void initializeMenubar();
    void initializeEditor();

    void readSettings();
    void writeSettings();

    const QString WINDOW_SIZE_KEY     = "window_size";
    const QString WINDOW_POSITION_KEY = "window_position";

    QMenu *viewMenu;

    Editor *editor;
    Functions *functions;

    Settings *settings = Settings::instance();
};

#endif // LUAD_MAINWINDOW_H
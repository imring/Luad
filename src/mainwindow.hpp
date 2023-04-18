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

#ifndef LUAD_MAINWINDOW_HPP
#define LUAD_MAINWINDOW_HPP

#include <QMainWindow>

#include "qhexedit.h"

#include "file.hpp"

class XrefMenu;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

    void closeEvent(QCloseEvent *event) override;

signals:
    void openFile(std::weak_ptr<File> file);

public slots:
    void openFileDialog();
    void closeFile();
    void jumpDialog();
    void initializeDisassembler(std::weak_ptr<File> file);
    void showXref(const QString &name, XrefMenu *menu);

private:
    void         initializeMenubar();
    QDockWidget *addDock(const QString &title, QWidget *widget, Qt::DockWidgetArea area = Qt::TopDockWidgetArea);
    void         removeDock(QDockWidget *&widget);

    QHexEdit *addHexEditor();

    std::shared_ptr<File> file;

    QAction *closeFileAction = nullptr;
    QAction *jumpAction      = nullptr;
    QMenu   *viewMenu        = nullptr;

    QDockWidget *disassembler = nullptr;
    QDockWidget *functions    = nullptr;
    QDockWidget *variables    = nullptr;
    QDockWidget *xref         = nullptr;
    QDockWidget *hexEditor    = nullptr;

    void readSettings();
    void writeSettings();
};

#endif // LUAD_MAINWINDOW_HPP
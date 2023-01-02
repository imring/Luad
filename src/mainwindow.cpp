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

#include "mainwindow.hpp"

MainWindow::MainWindow(QWidget *parent) : QMainWindow{parent} {
    setWindowTitle("Luad");

    readSettings();

    initializeMenubar();
    initializeEditor();
}

void MainWindow::initializeMenubar() {
    QMenu   *fileMenu = menuBar()->addMenu(tr("&File"));
    QAction *openFile = new QAction{"&Open", this};
    openFile->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_O});
    fileMenu->addAction(openFile);

    fileMenu->addSeparator();

    QAction *exit = new QAction{"&Exit", this};
    exit->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_Q});
    fileMenu->addAction(exit);

    QMenu   *editMenu = menuBar()->addMenu(tr("&Edit"));
    QAction *jumpTo   = new QAction{"&Jump to address", this};
    jumpTo->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_G});
    editMenu->addAction(jumpTo);

    viewMenu = menuBar()->addMenu(tr("&View"));

    connect(openFile, &QAction::triggered, this, &MainWindow::open);
    connect(exit, &QAction::triggered, this, &QCoreApplication::exit);
    connect(jumpTo, &QAction::triggered, this, &MainWindow::jumpTo);
}

void MainWindow::initializeEditor() {
    QFont font;
    font.setFamily("Courier New");
    font.setFixedPitch(true);
    font.setPointSize(10);

    QDockWidget *dock = new QDockWidget(tr("Disassembly"), this);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    editor = new Editor(this);
    editor->setFont(font);
    dock->setWidget(editor);
    addDockWidget(Qt::TopDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());

    dock = new QDockWidget(tr("Functions"), this);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    functions = new Functions(editor);
    dock->setWidget(functions);
    addDockWidget(Qt::TopDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());
}

void MainWindow::readSettings() {
    const QVariant size = settings->value(WINDOW_SIZE_KEY, QSize{800, 600});
    resize(size.toSize());

    const QVariant pos = settings->value(WINDOW_POSITION_KEY, QPoint{40, 40});
    move(pos.toPoint());
}

void MainWindow::writeSettings() {
    settings->setValue(WINDOW_SIZE_KEY, size());
    settings->setValue(WINDOW_POSITION_KEY, pos());
}

void MainWindow::open() {
    const QString path = QFileDialog::getOpenFileName(this, tr("Open"), "", tr("Compiled lua script (*.luac)"));
    if (path.isEmpty())
        return;
    editor->open(path);
}

void MainWindow::jumpTo() {
    if (!editor->ptrinfo()) { // script is not open
        return;
    }

    QInputDialog dialog{this};
    dialog.setWindowTitle("Jump to...");
    dialog.setLabelText("Enter the address in hex:");

    bool    ok;
    QString text = QInputDialog::getText(this, tr("Jump to..."), tr("Enter the address in hex:"), QLineEdit::Normal, {}, &ok);
    if (!ok) {
        return;
    }

    const int addr = text.toInt(&ok, 16);
    if (!ok) {
        QMessageBox::warning(this, "Warning", "Invalid address.");
        return;
    }

    ok = editor->jump(addr);
    if (!ok) {
        QMessageBox::warning(this, "Warning", "Invalid address.");
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    writeSettings();
}
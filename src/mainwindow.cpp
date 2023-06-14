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

#include "mainwindow.hpp"

#include <QMenuBar>
#include <QDockWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QCoreApplication>

#include "xrefmenu.hpp"
#include "settings.hpp"
#include "functions.hpp"
#include "variables.hpp"
#include "disassembler.hpp"

MainWindow::MainWindow(QWidget *parent) : QMainWindow{parent}, file{std::make_shared<File>()} {
    setWindowTitle("Luad");

    readSettings();
    initializeMenubar();

    connect(this, &MainWindow::openFile, this, &MainWindow::initializeDisassembler);
    connect(this, &MainWindow::openFile, LuaPluginManager::instance(), &LuaPluginManager::openFile);
    connect(LuaPluginManager::instance(), &LuaPluginManager::onMessage, this, &MainWindow::onMessage);

    LuaPluginManager::instance()->setParent(this);
    LuaPluginManager::instance()->loadPlugins();
}

MainWindow::~MainWindow() {
    disconnect(LuaPluginManager::instance(), &LuaPluginManager::onMessage, this, &MainWindow::onMessage);
}

void MainWindow::openFileDialog() {
    if (file->is_opened()) {
        closeFile();
    }
    const QString path = QFileDialog::getOpenFileName(this, tr("Open"), "", tr("Compiled lua script (*.luac)"));
    if (!path.isEmpty() && file->open(path)) {
        closeFileAction->setEnabled(true);
        jumpAction->setEnabled(true);

        QFileInfo fi{path};
        setWindowTitle(QString{"Luad - %1"}.arg(fi.fileName()));

        emit openFile(file);
    }
}

void MainWindow::closeFile() {
    setWindowTitle("Luad");
    file->close();
    closeFileAction->setEnabled(false);
    jumpAction->setEnabled(false);

    removeDock(variables);
    removeDock(functions);
    removeDock(disassembler);
    removeDock(hexEditor);
    removeDock(pluginLogs);
    if (xref) {
        removeDock(xref);
    }
}

void MainWindow::jumpDialog() {
    if (!file->is_opened()) { // script is not open
        return;
    }

    QInputDialog dialog{this};
    dialog.setWindowTitle("Jump to...");
    dialog.setLabelText("Enter the address in hex or the variable name:");

    bool    ok;
    QString text = QInputDialog::getText(this, tr("Jump to..."), tr("Enter the address in hex:"), QLineEdit::Normal, {}, &ok);
    if (!ok) {
        return;
    }

    Disassembler *disasm = qobject_cast<Disassembler *>(disassembler->widget());
    if (disasm->jump(text.toStdString())) {
        return;
    }

    const int addr = text.toInt(&ok, 16);
    if (!ok || !disasm->jump(addr)) {
        QMessageBox::warning(this, "Warning", "Invalid address or variable name.");
        return;
    }
}

void MainWindow::initializeDisassembler(std::weak_ptr<File> file) {
    Disassembler *disasm = new Disassembler{this, file};
    disassembler         = addDock(tr("Disassembler"), disasm, Qt::RightDockWidgetArea);
    QHexEdit *hex        = addHexEditor();
    hexEditor            = addDock(tr("Hex Editor"), hex, Qt::RightDockWidgetArea);
    tabifyDockWidget(disassembler, hexEditor);

    Functions *funcs = new Functions{disasm, file};
    functions        = addDock(tr("Functions"), funcs, Qt::LeftDockWidgetArea);
    Variables *vars  = new Variables{disasm, file};
    variables        = addDock(tr("Variables"), vars, Qt::LeftDockWidgetArea);
    tabifyDockWidget(functions, variables);
    resizeDocks({functions, variables}, {static_cast<int>(width() * 0.25), height()}, Qt::Horizontal);

    connect(disasm, &Disassembler::showXref, this, &MainWindow::showXref);

    // disasm -> hex
    connect(disasm, &Disassembler::cursorPositionChanged, [&] {
        if (hexEditor->isVisible()) {
            auto       hex    = static_cast<QHexEdit *>(hexEditor->widget());
            const auto disasm = static_cast<Disassembler *>(disassembler->widget());
            if (disasm && hex) {
                // * 2 for byte jump
                hex->setCursorPosition(disasm->getCurrentAddress() * 2);
            }
        }
    });
    // hex -> disasm
    connect(hex, &QHexEdit::currentAddressChanged, [&](qint64 address) {
        if (disassembler->isVisible()) {
            if (auto disasm = static_cast<Disassembler *>(disassembler->widget())) {
                if (address != disasm->getCurrentAddress()) {
                    disasm->jump(address);
                }
            }
        }
    });

    QPlainTextEdit *logsEdit = new QPlainTextEdit{this};
    logsEdit->setReadOnly(true);
    logsEdit->setPlainText(logs);
    pluginLogs = addDock(tr("Plugin logs"), logsEdit, Qt::RightDockWidgetArea);
}

void MainWindow::showXref(const QString &name, XrefMenu *menu) {
    if (xref) {
        xref->setWidget(menu);
        xref->setWindowTitle(name);
    } else {
        xref = addDock(name, menu, Qt::RightDockWidgetArea);
        tabifyDockWidget(disassembler, xref);
    }
    xref->show();
    xref->raise();
}

void MainWindow::onMessage(std::string_view text) {
    if (!logs.isEmpty()) {
        logs.append('\n');
    }
    logs.append(QString::fromUtf8(text.data(), text.size()));

    if (pluginLogs) {
        auto logsEdit = qobject_cast<QPlainTextEdit *>(pluginLogs->widget());
        logsEdit->setPlainText(logs);
    }
}

void MainWindow::initializeMenubar() {
    QMenu   *fileMenu = menuBar()->addMenu(tr("&File"));
    QAction *openFile = new QAction{"&Open", this};
    openFile->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_O});
    fileMenu->addAction(openFile);

    closeFileAction = new QAction{"&Close", this};
    closeFileAction->setEnabled(false);
    fileMenu->addAction(closeFileAction);

    fileMenu->addSeparator();

    QAction *exit = new QAction{"&Exit", this};
    exit->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_Q});
    fileMenu->addAction(exit);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    jumpAction      = new QAction{"&Jump to address", this};
    jumpAction->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_G});
    jumpAction->setEnabled(false);
    editMenu->addAction(jumpAction);

    viewMenu = menuBar()->addMenu(tr("&View"));

    connect(openFile, &QAction::triggered, this, &MainWindow::openFileDialog);
    connect(closeFileAction, &QAction::triggered, this, &MainWindow::closeFile);
    connect(exit, &QAction::triggered, this, &QCoreApplication::exit);
    connect(jumpAction, &QAction::triggered, this, &MainWindow::jumpDialog);
}

QDockWidget *MainWindow::addDock(const QString &title, QWidget *widget, Qt::DockWidgetArea area) {
    QDockWidget *dockWidget = new QDockWidget{title, this};
    dockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
    dockWidget->setWidget(widget);
    addDockWidget(area, dockWidget);
    viewMenu->addAction(dockWidget->toggleViewAction());
    return dockWidget;
}

void MainWindow::removeDock(QDockWidget *&widget) {
    viewMenu->removeAction(widget->toggleViewAction());
    removeDockWidget(widget);
    widget = nullptr;
}

QHexEdit *MainWindow::addHexEditor() {
    const std::vector<dislua::uchar> bytes = file->dump_info->info->buf.copy_data();
    QByteArray                       buffer(std::bit_cast<const char *>(bytes.data()), bytes.size());

    QHexEdit *hexEdit = new QHexEdit{this};
    hexEdit->setReadOnly(true);
    hexEdit->setData(buffer);
    return hexEdit;
}

// settings
void MainWindow::readSettings() {
    Settings      *settings = Settings::instance();
    const QVariant size     = settings->value(Settings::windowSizeKey, QSize{800, 600});
    resize(size.toSize());

    const QVariant pos = settings->value(Settings::windowPositionKey, QPoint{40, 40});
    move(pos.toPoint());
}

void MainWindow::writeSettings() {
    Settings *settings = Settings::instance();
    settings->setValue(Settings::windowSizeKey, size());
    settings->setValue(Settings::windowPositionKey, pos());
}

void MainWindow::closeEvent(QCloseEvent *event) {
    writeSettings();
}

void MainWindow::highlight(std::size_t from, std::size_t to, QColor color) {
    Disassembler *disasm = qobject_cast<Disassembler *>(disassembler->widget());
    if (disasm) {
        disasm->highlight(from, to, color);
    }
}

MainWindow *MainWindow::instance() {
    static MainWindow singleton;
    return &singleton;
}
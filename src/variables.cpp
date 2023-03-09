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

#include "variables.hpp"

#include <QHeaderView>

#include "disassembler.hpp"

Variables::Variables(Disassembler *disasm, std::weak_ptr<File> file) : QTableWidget{disasm}, disassembler{disasm}, file{file} {
    verticalHeader()->hide();
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    setColumnCount(4);
    QStringList header;
    header << "Name"
           << "Start"
           << "End"
           //    << "Type"
           << "Located in";
    setHorizontalHeaderLabels(header);

    update();

    connect(this, &QTableWidget::cellDoubleClicked, this, &Variables::jump);
}

void Variables::update() {
    clearContents();

    auto ptr = file.lock();
    if (!ptr || !ptr->is_opened()) {
        return;
    }

    const auto &divs = ptr->dump_info->divs;
    int         rows = 0;
    setRowCount(rows);
    for (int i = 2; i < divs.additional.size(); i++) { // "i = 2" to exclude compiler & header info
        const bclist::div &div = divs.additional[i];

        int prev = -1;
        for (int l = 0; l < div.additional.size(); l++) {
            const bclist::div &d = div.additional[l];
            for (int k = 0; k < d.lines.size(); k++) {
                setRowCount(rows + 1);
                const auto &val = d.lines[k];
                if (val.key.empty())
                    continue;

                QTableWidgetItem *name = new QTableWidgetItem{QString::fromStdString(val.key)};
                name->setFlags(name->flags() & ~Qt::ItemIsEditable);
                setItem(rows, 0, name);

                QTableWidgetItem *start = new QTableWidgetItem{QStringLiteral("%1").arg(val.from, 8, 16, QLatin1Char('0'))};
                start->setFlags(start->flags() & ~Qt::ItemIsEditable);
                setItem(rows, 1, start);

                QTableWidgetItem *end = new QTableWidgetItem{QStringLiteral("%1").arg(val.to, 8, 16, QLatin1Char('0'))};
                end->setFlags(end->flags() & ~Qt::ItemIsEditable);
                setItem(rows, 2, end);

                // QTableWidgetItem *type = new QTableWidgetItem{QStringLiteral("%1").arg(val.to, 8, 16, QLatin1Char('0'))};
                // type->setFlags(type->flags() & ~Qt::ItemIsEditable);
                // setItem(rows, 3, type);

                QTableWidgetItem *located = new QTableWidgetItem{QString::fromStdString(div.key)};
                located->setFlags(located->flags() & ~Qt::ItemIsEditable);
                setItem(rows, 3, located);

                rows++;
            }
        }
    }
}

void Variables::jump(int row) {
    const QTableWidgetItem *name    = item(row, 0);
    const QString           namestr = name->data(0).toString();
    if (disassembler) {
        disassembler->jump(namestr.toStdString());
    }
}
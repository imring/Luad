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

#include "xrefmenu.hpp"

#include <QHeaderView>

#include "utils.hpp"
#include "disassembler.hpp"

XrefMenu::XrefMenu(Disassembler *disasm, std::weak_ptr<File> file, std::size_t ref) : QTableWidget{disasm}, disassembler{disasm}, file{file} {
    verticalHeader()->hide();
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    setColumnCount(2);
    QStringList header;
    header << "Address"
           << "Line";
    setHorizontalHeaderLabels(header);

    update(ref);

    connect(this, &QTableWidget::cellDoubleClicked, this, &XrefMenu::jump);
}

void XrefMenu::update(std::size_t ref) {
    clearContents();

    auto ptr = file.lock();
    if (!ptr || !ptr->is_opened()) {
        return;
    }

    const auto it = ptr->dump_info->refs.find(ref);
    if (it == ptr->dump_info->refs.end()) {
        return;
    }

    int i = 0;

    const std::function<void(const bclist::div &divs)> findRefInDiv = [&](const bclist::div &divs) {
        while (true) {
            if (i == it->second.size()) {
                return;
            }

            const std::size_t idx = utils::line_by_addr(divs.lines, it->second[i], true);
            if (idx == bclist::max_line) {
                break;
            }

            QTableWidgetItem *addr = new QTableWidgetItem{QStringLiteral("%1").arg(it->second[i], 8, 16, QLatin1Char('0'))};
            addr->setFlags(addr->flags() & ~Qt::ItemIsEditable);
            setItem(i, 0, addr);

            QTableWidgetItem *li = new QTableWidgetItem{QString::fromStdString(divs.lines[idx].text)};
            li->setFlags(li->flags() & ~Qt::ItemIsEditable);
            setItem(i, 1, li);

            i++;
        }

        for (const auto &add: divs.additional) {
            findRefInDiv(add);
        }
    };

    const auto &divs = ptr->dump_info->divs;
    setRowCount(it->second.size());
    for (int i = 2; i < divs.additional.size(); i++) { // "i = 2" to exclude compiler & header info
        findRefInDiv(divs.additional[i]);
    }
}

void XrefMenu::jump(int row) {
    const QString addrstr = item(row, 0)->data(0).toString();
    const int     addr    = addrstr.toInt(nullptr, 16);

    if (disassembler) {
        disassembler->jump(addr, true);
    }
}
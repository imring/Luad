#include "variables.hpp"
#include "editor.hpp"

Variables::Variables(Editor *editor) : QTableWidget{editor}, editor{editor} {
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

    connect(editor, &Editor::onOpenFile, this, &Variables::update);
    connect(this, &QTableWidget::cellDoubleClicked, this, &Variables::jump);
}

void Variables::update() {
    clearContents();

    if (!editor || !editor->ptrinfo())
        return;

    const auto ptr = editor->ptrinfo();
    int rows = 0;
    setRowCount(rows);
    for (int i = 2; i < ptr->divs.additional.size(); i++) { // "i = 2" to exclude compiler & header info
        const bclist::div &div = ptr->divs.additional[i];

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
    const QTableWidgetItem *i    = item(row, 1); // start
    const QString           val  = i->data(0).toString();
    const int               addr = val.toInt(nullptr, 16);

    editor->jump(addr);
}
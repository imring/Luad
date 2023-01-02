#include "functions.hpp"
#include "editor.hpp"

Functions::Functions(Editor *editor) : QTableWidget{editor}, editor{editor} {
    verticalHeader()->hide();
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    setColumnCount(3);
    QStringList header;
    header << "Name"
           << "Start"
           << "End";
    setHorizontalHeaderLabels(header);

    update();

    connect(editor, &Editor::onOpenFile, this, &Functions::update);
    connect(this, &QTableWidget::cellDoubleClicked, this, &Functions::jump);
}

void Functions::update() {
    clearContents();

    if (!editor || !editor->ptrinfo())
        return;

    const auto ptr = editor->ptrinfo();
    setRowCount(ptr->divs.additional.size() - 2);
    for (int i = 2; i < ptr->divs.additional.size(); i++) { // "i = 2" to exclude compiler & header info
        const bclist::div &div  = ptr->divs.additional[i];
        std::string        head = div.header.substr(0, div.header.size() - 3); // remove " do"

        QTableWidgetItem *name = new QTableWidgetItem{QString::fromStdString(head)};
        name->setFlags(name->flags() & ~Qt::ItemIsEditable);
        setItem(i - 2, 0, name);

        QTableWidgetItem *start = new QTableWidgetItem{QStringLiteral("%1").arg(div.start(), 8, 16, QLatin1Char('0'))};
        start->setFlags(start->flags() & ~Qt::ItemIsEditable);
        setItem(i - 2, 1, start);

        QTableWidgetItem *end = new QTableWidgetItem{QStringLiteral("%1").arg(div.end(), 8, 16, QLatin1Char('0'))};
        end->setFlags(end->flags() & ~Qt::ItemIsEditable);
        setItem(i - 2, 2, end);
    }
}

void Functions::jump(int row) {
    const QTableWidgetItem *i    = item(row, 1); // start
    const QString           val  = i->data(0).toString();
    const int               addr = val.toInt(nullptr, 16);

    editor->jump(addr);
}
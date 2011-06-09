#include "SpreadSheet.h"

SpreadSheet::SpreadSheet(int rows, int columns, QWidget *parent) : QTableWidget(parent)
{
    (rows < 1)?setRowCount(600):setRowCount(rows);
    (columns < 1)?setColumnCount(6):setColumnCount(columns);

    setItemPrototype(new Cell());
    setSelectionMode(ContiguousSelection);
    setContextMenuPolicy(Qt::ActionsContextMenu);

    refresh_timer = new QTimer(this);
    refresh_timer->start(5000);

    timestamps = new QStringList();

    connect(this, SIGNAL(itemChanged(QTableWidgetItem *)),
            this, SLOT(somethingChanged(QTableWidgetItem *)));

    clear();
}

SpreadSheet::~SpreadSheet()
{
    refresh_timer->stop();
    delete refresh_timer;
    delete timestamps;
}

bool SpreadSheet::printSpreadSheet(const QString &fileName) const
{

    if (fileName.length() == 0)
        return false;
    if (!fileName.contains(".pdf", Qt::CaseSensitive))
        return false;

    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    int width = printer.width();
    QPainter painter;

    if (!painter.begin(&printer))
        return false;

    int x = 0;
    int y = 0;
    int incrY = 0;
    int row_height = 20;
    for (int i=0; i<getNonzeroRowCount(0); i++)
    {
        x = 0;
        incrY = 0;
        for (int j=0; j<columnCount(); j++)
        {
            QRect required = QRect();
            QRect r = QRect(x, y, width/columnCount(), row_height);
            painter.drawRect(r);
            QString text = (cell(i,j) == NULL)?"":cell(i,j)->text();
            painter.setFont((cell(i,j) == NULL)?font():cell(i,j)->font());
            painter.drawText(r, Qt::AlignCenter | Qt::TextWordWrap,
                             text, &required);

            if (required.height() > r.height())
            {
                painter.eraseRect(r);
                incrY = required.height() - r.height();
                int aux_height = row_height;
                row_height = required.height();
                r.setHeight(row_height);
                painter.drawRect(r);
                painter.drawText(r, Qt::AlignCenter | Qt::TextWordWrap, text);

                int aux_x = x;
                for (int k=j-1; k>=0; k--)
                {
                    aux_x -= width/columnCount();
                    r = QRect(aux_x, y, width/columnCount(), aux_height);
                    painter.eraseRect(r);
                    r.setHeight(row_height);
                    painter.drawRect(r);

                    text = (cell(i,k) == NULL)?"":cell(i,k)->text();
                    painter.setFont((cell(i,k) == NULL)?font():cell(i,k)->font());
                    painter.drawText(r, Qt::AlignCenter | Qt::TextWordWrap,
                                                 text, &required);
                }
            }
            x += width/columnCount();
        }
        row_height = 20;
        y += row_height + incrY;
    }
    painter.end();
    return true;
}

QString SpreadSheet::currentLocation() const
{
    return QChar('A' + currentColumn()) + QString::number(currentRow() + 1);
}

QString SpreadSheet::getLocation(int row, int column) const
{
    if ((row >= rowCount()) || (column >= columnCount()))
        return "";
    return QChar('A' + column) + QString::number(row + 1);
}

QString SpreadSheet::currentFormula() const
{
    return formula(currentRow(), currentColumn());
}

QTableWidgetSelectionRange SpreadSheet::selectedRange() const
{
    QList<QTableWidgetSelectionRange> ranges = selectedRanges();
    if (ranges.isEmpty())
        return QTableWidgetSelectionRange();
    return ranges.first();
}

QTimer *SpreadSheet::getTimer() const
{
    return this->refresh_timer;
}

void SpreadSheet::replaceTimestamp(int index,
                      const QString &newVal) const
{
    if (index < timestamps->size())
        timestamps->replace(index, newVal);
}

void SpreadSheet::addTimestamp(const QString &ts) const
{
    timestamps->append(ts);
}

int SpreadSheet::timestampCount() const
{
    return timestamps->size();
}

QString SpreadSheet::getTimestamp(int index) const
{
    if (index >= timestamps->size())
        return "";
    else
        return timestamps->at(index);
}

void SpreadSheet::setCellsSize(const QSize &size)
{
    if (size.height() >= 20)
        for (int i=0; i<rowCount(); i++)
            setRowHeight(i, size.height());
    if (size.width() >= 50)
        for (int i=0; i<columnCount(); i++)
            setColumnWidth(i, size.width());
}

void SpreadSheet::setFormula(const QString &formula)
{
    QTableWidgetSelectionRange range = selectedRange();
    if (range.columnCount() * range.rowCount() == 0)
        return;
    if (range.columnCount() * range.rowCount() == 1)
    {
        Cell *c = new Cell(formula);
        connect(c, SIGNAL(getLink(QString,int,int,const QTableWidgetItem*)),
                this, SLOT(emitGetLink(QString,int,int,const QTableWidgetItem*)));
        connect(c, SIGNAL(invalidFormula(QString)),
                this, SIGNAL(invalidFormula(QString)));
        c->setData(Qt::EditRole, formula);
        setItem(range.topRow(), range.leftColumn(), c);
    }
    else
    {
        int pos = 0;
        int start_pos = 0;
        QRegExp pattern("[A-Z][0-9]{1,3}");
        QStringList matches = QStringList();
        QList <int> matches_pos = QList <int>();
        QList <int> matches_len = QList <int>();
        while ((pos = formula.indexOf(pattern, start_pos)) != -1)
        {
            start_pos = pos + 1;
            matches.append(formula.mid(pos, pattern.matchedLength()));
            matches_pos.append(pos);
            matches_len.append(pattern.matchedLength());
        }

        for (int i=0; i<range.rowCount(); i++)
        {
            for (int j=0; j<range.columnCount(); j++)
            {
                QString f = QString(formula);
                for (int m=0; m<matches.length(); m++)
                {
                    QString id = QString(matches.at(m));
                    int column = ((int)(id[0].toAscii()))-65;
                    int row = id.right(id.length()-1).toInt()-1;

                    QString new_id = getLocation(row+i, column+j);
                    if (new_id.length() > 0)
                        f.replace(matches_pos.at(m), matches_len.at(m), new_id);
                }

                Cell *c = new Cell(f);
                connect(c, SIGNAL(getLink(QString,int,int,const QTableWidgetItem*)),
                        this, SLOT(emitGetLink(QString,int,int,const QTableWidgetItem*)));
                connect(c, SIGNAL(invalidFormula(QString)),
                        this, SIGNAL(invalidFormula(QString)));
                c->setData(Qt::EditRole, f);
                setItem(range.topRow()+i, range.leftColumn()+j, c);
            }
        }
    }
}

QList<int> SpreadSheet::selectedColumns()
{
    int current_col = -1;
    QList<int> aux = QList<int>();

    QList<QTableWidgetItem*> columns = selectedItems();
    for (int i=0; i<columns.length(); i++)
    {
        current_col = column(columns.at(i));
        if (!aux.contains(current_col))
            aux.append(current_col);
    }

    return aux;
}

int SpreadSheet::getNonzeroRowCount(int column) const
{
    int count = 0;
    QTableWidgetItem *aux = new QTableWidgetItem();
    for (int i=0; i<rowCount(); i++)
    {
        aux = item(i, column);
        if (aux == NULL)
            continue;
        else if (aux->text() != "")
            count++;
    }
    delete aux;
    return count;
}

void SpreadSheet::clear()
{
    setRowCount(rowCount());
    setColumnCount(columnCount());

    for (int i = 0; i < columnCount(); ++i)
    {
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setText(QString(QChar('A' + i)));
        this->setHorizontalHeaderItem(i, item);
    }

    setCurrentCell(0, 0);
}

Cell* SpreadSheet::cell(int row, int column) const
{
    if (row < rowCount() && column < columnCount())
        return (Cell *)item(row, column);
    else
        return 0;
}

QString SpreadSheet::text(int row, int column) const
{
    Cell *c = cell(row, column);
    if (c)
        return c->text();
    else
        return "";
}

QString SpreadSheet::formula(int row, int column) const
{
    Cell *c = cell(row, column);
    if (c)
        return c->formula();
    else
        return "";
}

void SpreadSheet::setFormula(int row, int column,
                             const QString &formula)
{
    Cell *c = new Cell(formula);
    connect(c, SIGNAL(getLink(QString,int,int,const QTableWidgetItem*)),
            this, SLOT(emitGetLink(QString,int,int,const QTableWidgetItem*)));
    connect(c, SIGNAL(invalidFormula(QString)),
            this, SIGNAL(invalidFormula(QString)));
    c->setData(Qt::EditRole, formula);
    setItem(row, column, c);
}

void SpreadSheet::cut()
{
    copy();
    del();
}

void SpreadSheet::copy()
{
    QTableWidgetSelectionRange range = selectedRange();
    QString str;

    for (int i=0; i<range.rowCount(); i++)
    {
        for (int j=0; j<range.columnCount(); j++)
        {
            str += formula(range.topRow()+i, range.leftColumn()+j);
            str += "\t";
        }
        str.chop(1);
        str += "\n";
    }
    str.chop(1);
    qDebug() << str;
    QApplication::clipboard()->setText(str);
}

void SpreadSheet::paste()
{
    QTableWidgetSelectionRange range = selectedRange();
    QString str = QApplication::clipboard()->text();
    QStringList rows = str.split('\n');
    int numRows = rows.count();
    int numColumns = rows.first().count('\t') + 1;

    if (range.rowCount() * range.columnCount() != 1
            && (range.rowCount() != numRows
                || range.columnCount() != numColumns))
    {
        QMessageBox::information(this, tr("SpreadSheet"),
                tr("Invalid size"));
        return;
    }

    for (int i=0; i<numRows; i++)
    {
        QStringList columns = rows[i].split('\t');
        for (int j=0; j<numColumns; j++)
        {
            int row = range.topRow() + i;
            int column = range.leftColumn() + j;
            if (row < rowCount() && column < columnCount())
                setFormula(row, column, columns[j]);
        }
    }
    somethingChanged(currentItem());
}

void SpreadSheet::del()
{
    QList<QTableWidgetItem *> items = selectedItems();
    if (!items.isEmpty())
    {
        for (int i=0; i<items.length(); i++)
            if (items[i] != 0)
                items[i]->setData(Qt::EditRole, "");
        somethingChanged(currentItem());
    }
}

void SpreadSheet::selectCurrentRow()
{
    selectRow(currentRow());
}

void SpreadSheet::selectCurrentColumn()
{
    selectColumn(currentColumn());
}

void SpreadSheet::findNext(const QString &str, Qt::CaseSensitivity cs)
{
    int row = currentRow();
    int column = currentColumn() + 1;

    while (row < rowCount())
    {
        while (column < columnCount())
        {
            if (text(row, column).contains(str, cs))
            {
                clearSelection();
                setCurrentCell(row, column);
                activateWindow();
                return;
            }
            ++column;
        }
        column = 0;
        ++row;
    }
}

void SpreadSheet::findPrevious(const QString &str,
                               Qt::CaseSensitivity cs)
{
    int row = currentRow();
    int column = currentColumn() - 1;

    while (row >= 0)
    {
        while (column >= 0)
        {
            if (text(row, column).contains(str, cs))
            {
                clearSelection();
                setCurrentCell(row, column);
                activateWindow();
                return;
            }
            --column;
        }
        column = columnCount() - 1;
        --row;
    }
}

void SpreadSheet::addColumns(int columns)
{
    for (int i=0; i<columns; i++)
        insertColumn(columnCount()+i);
    setColumnCount(columnCount() + columns - 1);
    clear();
}

void SpreadSheet::addRows(int rows)
{
    for (int i=0; i<rows; i++)
        insertRow(rowCount()+i);
    setRowCount(rowCount() + rows - 1);
}

void SpreadSheet::removeColumns(const QList <int> column_ids)
{
    for (int i=0; i<column_ids.length(); i++)
        removeColumn(column_ids.at(i));
    clear();
}

void SpreadSheet::somethingChanged(QTableWidgetItem *cell)
{
    if (cell == 0)
        return;
    QString aux = QString("%1\n%2\n%3").
                    arg(cell->row()).
                    arg(cell->column()).
                    arg(cell->data(Qt::EditRole).toString());
    emit modified(aux);
}

void SpreadSheet::loadData(const QStringList &data)
{
    if (data.length() == 0)
        return;
    QStringList aux;
    for (int i=0; i<data.length(); i++)
    {
        aux = ((QString)data.at(i)).split('\n');
        for (int j=0; j<aux.length(); j++)
            this->setFormula(i, j, aux[j]);
    }
}

void SpreadSheet::emitGetLink(const QString &table, int r,
                 int c, const QTableWidgetItem* cell)
{
    emit getLink(table, r, c, column(cell), row(cell));
}

#include "SpreadSheet.h"

SpreadSheet::SpreadSheet(int rows, int columns, QWidget *parent) : QTableWidget(parent)
{
    (rows < 1)?setRowCount(100):setRowCount(rows);
    (columns < 1)?setColumnCount(6):setColumnCount(columns);

    setItemPrototype(new Cell());
    setSelectionMode(ContiguousSelection);
    setContextMenuPolicy(Qt::ActionsContextMenu);

    refresh_timer = new QTimer(this);
    refresh_timer->start(5000);

    timestamps = new QMap<int,QString>();

    connect(this, SIGNAL(itemChanged(QTableWidgetItem *)),
            this, SLOT(somethingChanged(QTableWidgetItem *)));

    connect(this->horizontalHeader(), SIGNAL(sectionResized(int,int,int)),
            this, SIGNAL(columnResize(int,int,int)));
    connect(this->verticalHeader(), SIGNAL(sectionResized(int,int,int)),
            this, SIGNAL(rowResize(int,int,int)));
    connect(this, SIGNAL(itemSelectionChanged()),
            this, SLOT(currentSelectionChanged()));

    clear();
}

SpreadSheet::~SpreadSheet()
{
    for (int i=0; i<rowCount(); i++)
        for (int j=0; j<columnCount(); j++)
            if (cell(i,j))
                delete cell(i,j);
    refresh_timer->stop();
    delete refresh_timer;
    delete timestamps;
}

bool SpreadSheet::printSpreadSheet(const QString &fileName) const
{
    QString fname = fileName;
    if (fname.length() == 0)
        return false;
    if (fname.right(4) != ".pdf")
        fname.append(".pdf");
    
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fname);
    int width = printer.width();
    int sWidth = 0;
    for (int i=0; i<columnCount(); i++)
        sWidth += columnWidth(i);
    int diff = (width - sWidth) / columnCount();
    
    QPainter painter;
    
    if (!painter.begin(&printer))
        return false;
    
    int x = 0;
    int y = 0;
    int incrY = 0;
    int row_height = 20;
    QBrush brush = painter.brush();
    painter.setPen(QPen(QBrush(Qt::black), 0.5));
    QPen pen = painter.pen();
    
    for (int i=-1; i<rowCount(); i++)
    {
        x = 0;
        incrY = 0;
        for (int j=0; j<columnCount(); j++)
        {
            QRect required = QRect();
            QRect r = QRect(x, y, columnWidth(j)+diff, row_height);
            painter.setBrush(cell(i,j)->background());
            painter.drawRect(r);
            painter.setBrush(brush);
            QString text = (!cell(i,j))?"":cell(i,j)->text();
            painter.setFont(!(cell(i,j))?font():cell(i,j)->font());
            painter.setPen(cell(i,j)->foreground().color());
            painter.drawText(r, cell(i,j)->textAlignment() | Qt::TextWrapAnywhere,
                             text, &required);
            painter.setPen(pen.color());
            
            if (required.height() > r.height())
            {
                painter.eraseRect(r.x(), r.y(), r.width(), r.height()+1);
                incrY = required.height() - r.height();
                int aux_height = row_height;
                row_height = required.height();
                r.setHeight(row_height);
                painter.setBrush(cell(i,j)->background());
                painter.drawRect(r);
                painter.setBrush(brush);
                painter.setPen(cell(i,j)->foreground().color());
                painter.drawText(r, cell(i,j)->textAlignment() | Qt::TextWrapAnywhere, text);
                painter.setPen(pen.color());
                
                int aux_x = x;
                for (int k=j-1; k>=0; k--)
                {
                    aux_x -= columnWidth(k)+diff;
                    r = QRect(aux_x, y, columnWidth(k)+diff, aux_height+1);
                    painter.eraseRect(r);
                    r.setHeight(row_height);
                    painter.setBrush(cell(i,k)->background());
                    painter.drawRect(r);
                    painter.setBrush(brush);
                    
                    text = (!cell(i,k))?"":cell(i,k)->text();
                    painter.setFont(!(cell(i,k))?font():cell(i,k)->font());
                    painter.setPen(cell(i,k)->foreground().color());
                    painter.drawText(r, cell(i,k)->textAlignment() | Qt::TextWrapAnywhere,
                                     text);
                    painter.setPen(pen.color());
                }
            }
            x += columnWidth(j)+diff;
        }
        row_height = 20;
        if (y >= printer.height()-row_height)
        {
            printer.newPage();
            y = 0;
        }
        else
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

void SpreadSheet::setRefreshTime(int sec) const
{
    int msec = sec*1000;
    refresh_timer->setInterval(msec);
}

void SpreadSheet::replaceTimestamp(int index,
                      const QString &newVal) const
{
    timestamps->insert(index, newVal);
}

void SpreadSheet::addTimestamp(int index, const QString &ts) const
{
    timestamps->insert(index, ts);
}

int SpreadSheet::timestampCount() const
{
    return timestamps->size();
}

QString SpreadSheet::getTimestamp(int index) const
{
    return timestamps->value(index);
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
        Cell *c = cell(range.topRow(), range.leftColumn());
        if (!c)
        {
            c = new Cell();
            setItem(range.topRow(), range.leftColumn(), c);
        }
        connect(c, SIGNAL(getLink(QString,int,int,const QTableWidgetItem*)),
                this, SLOT(emitGetLink(QString,int,int,const QTableWidgetItem*)));
        connect(c, SIGNAL(invalidFormula(QString)),
                this, SIGNAL(invalidFormula(QString)));
        c->setData(Qt::EditRole, formula);
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

                Cell *c = cell(range.topRow()+i, range.leftColumn()+j);
                if (!c)
                {
                    c = new Cell();
                    setItem(range.topRow()+i, range.leftColumn()+j, c);
                }
                connect(c, SIGNAL(getLink(QString,int,int,const QTableWidgetItem*)),
                        this, SLOT(emitGetLink(QString,int,int,const QTableWidgetItem*)));
                connect(c, SIGNAL(invalidFormula(QString)),
                        this, SIGNAL(invalidFormula(QString)));
                c->setData(Qt::EditRole, f);
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
    qSort(aux);

    return aux;
}

QString SpreadSheet::headerText(int column)
{
    if (column >= 0 && column < columnCount())
    {
        QTableWidgetItem *item = horizontalHeaderItem(column);
        if (item)
            return item->text();
        else
            return QString(QChar('A' + column));
    }
    return "";
}

void SpreadSheet::setHeaderText(int column, const QString &text)
{
    if (column >= 0 && column < columnCount())
    {
        QTableWidgetItem *item = horizontalHeaderItem(column);
        if (!item)
        {
            item = new QTableWidgetItem();
            setHorizontalHeaderItem(column, item);
        }
        QString headerText = QString("%1").arg(QChar('A' + column));
        if (text != "")
            headerText.append(QString("\n%1").arg(text));
        item->setText(headerText);
    }
}

QFont SpreadSheet::currentFont() const
{
    QTableWidgetItem *item = currentItem();
    if (!item)
        return QApplication::font();
    
    return item->font();
}

void SpreadSheet::clear()
{
    setRowCount(rowCount());
    setColumnCount(columnCount());

    for (int i = 0; i < columnCount(); ++i)
    {
        QTableWidgetItem *item = horizontalHeaderItem(i);
        if (!item)
            item = new QTableWidgetItem();
        QString hText = item->text();
        item->setText(hText.replace(0, 1, QString(QChar('A' + i))));
        this->setHorizontalHeaderItem(i, item);
    }

    setCurrentCell(0, 0);
}

Cell* SpreadSheet::cell(int row, int column) const
{
    if (row < rowCount() && column < columnCount())
        if (row == -1)
        {
            Cell* c = (Cell *)horizontalHeaderItem(column);
            c->setTextAlignment(Qt::AlignCenter);
            return c;
        }
        else
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
    Cell *c = cell(row, column);
    if (!c)
    {
        c = new Cell();
        setItem(row, column, c);
    }
    connect(c, SIGNAL(getLink(QString,int,int,const QTableWidgetItem*)),
            this, SLOT(emitGetLink(QString,int,int,const QTableWidgetItem*)));
    connect(c, SIGNAL(invalidFormula(QString)),
            this, SIGNAL(invalidFormula(QString)));
    c->setData(Qt::EditRole, formula);
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

void SpreadSheet::setCurrentColumnHeaderText(const QString &text)
{
    int column = currentColumn();
    setHeaderText(column, text);
    
    emit columnHeaderTextChanged(column, text);
}

void SpreadSheet::setCurrentCellsFont(const QFont &f)
{
    QList<QTableWidgetItem*> items = selectedItems();
    for (int i=0; i<items.length(); i++)
        if (items[i])
            items[i]->setFont(f);
}

void SpreadSheet::setFontColor(const QColor &c)
{
    QList<QTableWidgetItem*> items = selectedItems();
    for (int i=0; i<items.length(); i++)
        if (items[i])
            items[i]->setForeground(QBrush(c));
}

void SpreadSheet::setBackgroundColor(const QColor &c)
{
    QList<QTableWidgetItem*> items = selectedItems();
    for (int i=0; i<items.length(); i++)
        if (items[i])
            items[i]->setBackgroundColor(c);
}

void SpreadSheet::setRowsSize(const QMap<int, int> size)
{
    QMapIterator<int, int> i(size);
    while (i.hasNext())
    {
        i.next();
        setRowHeight(i.key(), i.value());
    }
}

void SpreadSheet::setColumnsSize(const QMap<int, int> size)
{
    QMapIterator<int, int> i(size);
    while (i.hasNext())
    {
        i.next();
        setColumnWidth(i.key(), i.value());
    }
}

void SpreadSheet::setColumnsHeaderText(const QMap<int, QString> data)
{
    QMapIterator<int, QString> it(data);
    while (it.hasNext())
    {
        it.next();
        int column = it.key();
        QString text = it.value();
        QTableWidgetItem *item = horizontalHeaderItem(column);
        if (!item)
        {
            item = new QTableWidgetItem();
            setHorizontalHeaderItem(column, item);
        }
        QString headerText = QString("%1").arg(QChar('A' + column));
        if (text != "")
            headerText.append(QString("\n%1").arg(text));
        item->setText(headerText);
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
    for (int i=column_ids.length()-1; i>=0; i--)
    {
        removeColumn(column_ids.at(i));
        QStringList headerTxt = headerText(column_ids.at(i)).split('\n');
        if (headerTxt.length() > 0)
        {
            headerTxt.removeAt(0);
            setHeaderText(column_ids.at(i), headerTxt.join("\n"));
        }
    }
}

void SpreadSheet::setRights(const QList<int> columns)
{
    for (int col=0; col<columnCount(); col++)
        if (!columns.contains(col))
            for (int row=0; row<rowCount(); row++)
            {
                Cell *c = cell(row, col);
                if (!c)
                {
                    c = new Cell();
                    setItem(row, col, c);
                }
                if (c->flags() & Qt::ItemIsEditable)
                    c->setFlags(c->flags() ^ Qt::ItemIsEditable);
            }
        else
            for (int row=0; row<rowCount(); row++)
            {
                Cell *c = cell(row, col);
                if (!c)
                {
                    c = new Cell();
                    setItem(row, col, c);
                }
                if (!(c->flags() & Qt::ItemIsEditable))
                    c->setFlags(c->flags() | Qt::ItemIsEditable);
            }
}

void SpreadSheet::setSize(int rows, int columns)
{
    if (rows == 0 || columns == 0)
        return;
        
    setRowCount(rows);
    setColumnCount(columns);
}

void SpreadSheet::somethingChanged(QTableWidgetItem *cell)
{
    if (cell == 0)
        return;
    
    if (cell->font().family() == "")
        cell->setFont(QApplication::font());
    QByteArray cellData;
    QDataStream out(&cellData, QIODevice::WriteOnly);
    out << cell->font() << cell->foreground() << cell->background() << 
           cell->data(Qt::EditRole).toString();
    
    QString aux = QString("%1\n%2\n%3").
                    arg(cell->row()).
                    arg(cell->column()).
                    arg(QCA::arrayToHex(cellData));
    emit modified(aux);
}

void SpreadSheet::loadData(const QMap<int, QString> &data)
{
    QStringList aux;
    QMapIterator<int, QString> it(data);
    while (it.hasNext())
    {
        it.next();
        aux = ((QString)it.value()).split('\n');
        for (int c=0; c<aux.length(); c++)
        {
            QFont font;
            QBrush foreground, background;
            QString formula;
            
            QByteArray cellData = QCA::hexToArray(aux.at(c));
            QDataStream in(&cellData, QIODevice::ReadOnly);
            in >> font >> foreground >> background >> formula;
                    
            setFormula(it.key(), c, formula);
            QTableWidgetItem *item = this->item(it.key(), c);
            item->setFont(font);
            item->setForeground(foreground);
            item->setBackground(background);
        }
    }
}

void SpreadSheet::emitGetLink(const QString &table, int r,
                 int c, const QTableWidgetItem* cell)
{
    emit getLink(table, r, c, row(cell), column(cell));
}

void SpreadSheet::currentSelectionChanged()
{
    QTableWidgetItem *item = currentItem();
    if (!item)
        emit currentSelectionChanged(QApplication::font(), QBrush(Qt::white),
                                     QBrush(Qt::black));
    else if (item->font().family() == "")
        emit currentSelectionChanged(QApplication::font(), QBrush(Qt::white),
                                     QBrush(Qt::black));
    else
        emit currentSelectionChanged(item->font(), item->background(),
                                     item->foreground());
}

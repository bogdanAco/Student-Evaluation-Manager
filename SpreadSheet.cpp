#include "SpreadSheet.h"
#include "Cell.h"

SpreadSheet::SpreadSheet(int rows, int columns, QWidget *parent,
                         const DBManager *const mng) : 
    QTableWidget(parent)
{
    (rows < 1)?setRowCount(100):setRowCount(rows);
    (columns < 1)?setColumnCount(6):setColumnCount(columns);

    setItemPrototype(new Cell());
    setSelectionMode(ExtendedSelection);
    setContextMenuPolicy(Qt::ActionsContextMenu);
    this->mng = mng;

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
            r.setX(r.x()+2);
            r.setWidth(r.width()-4);
            painter.setBrush(brush);
            QString text = (!cell(i,j))?"":cell(i,j)->text();
            painter.setFont(!(cell(i,j))?font():cell(i,j)->font());
            painter.setPen(cell(i,j)->foreground().color());
            painter.drawText(r, cell(i,j)->textAlignment() | Qt::TextWrapAnywhere,
                             text, &required);
            painter.setPen(pen.color());
            r.setX(r.x()-2);
            r.setWidth(r.width()+4);
            
            if (required.height() > r.height())
            {
                painter.eraseRect(r.x(), r.y(), r.width(), r.height()+1);
                incrY = required.height() - r.height();
                int aux_height = row_height;
                row_height = required.height();
                r.setHeight(row_height);
                painter.setBrush(cell(i,j)->background());
                painter.drawRect(r);
                r.setX(r.x()+2);
                r.setWidth(r.width()-4);
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
                    r.setX(r.x()+2);
                    r.setWidth(r.width()-4);
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

bool SpreadSheet::selectionsEquals(const QMultiMap<int, int> &first, 
                                   const QMultiMap<int, int> &second)
{
    QList<int> firstUniqueKeys = first.uniqueKeys();
    QList<int> secondUniqueKeys = second.uniqueKeys();
    if (firstUniqueKeys.size() != secondUniqueKeys.size())
        return false;
    
    QList<int> firstColumnsSize = QList<int>();
    QListIterator<int> firstIt(firstUniqueKeys);
    while (firstIt.hasNext())
        firstColumnsSize.append(first.values(firstIt.next()).size());
    
    QList<int> secondColumnsSize = QList<int>();
    QListIterator<int> secondIt(secondUniqueKeys);
    while (secondIt.hasNext())
        secondColumnsSize.append(second.values(secondIt.next()).size());
    
    return qEqual(firstColumnsSize.begin(), firstColumnsSize.end(),
                  secondColumnsSize.begin());
}

QString SpreadSheet::currentFormula() const
{
    return formula(currentRow(), currentColumn());
}

QPair<int,int> SpreadSheet::getLocation(const QString &cellId)
{
    int column = ((int)(cellId.at(0).toAscii()))-65;
    int row = (cellId.mid(1).toInt())-1;
    
    if (row >= 0)
        return QPair<int,int>(row, column);
    else
        return QPair<int,int>(-1,-1);
}

QMultiMap<int,int> SpreadSheet::selectedItemIndexes() const
{
    QMultiMap<int,int> result = QMultiMap<int,int>();
    QMultiMap<int,int> aux = QMultiMap<int,int>();
    
    QModelIndexList idxs = selectedIndexes();
    QListIterator<QModelIndex> it(idxs);
    while (it.hasNext())
    {
        QModelIndex idx = it.next();
        aux.insert(idx.row(), idx.column());
    }
    
    QList<int> rows = aux.uniqueKeys();
    QListIterator<int> rowsIt(rows);
    while (rowsIt.hasNext())
    {
        int row = rowsIt.next();
        QList<int> columns = aux.values(row);
        qSort(columns.begin(), columns.end(), qGreater<int>());
        QListIterator<int> columnsIt(columns);
        while (columnsIt.hasNext())
            result.insert(row, columnsIt.next());
    }
    
    return result;
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

void SpreadSheet::setFormula(const QString &formula, 
                             const QMultiMap<int, int> &selection)
{
    QRegExp pattern("[A-Z][1-9][0-9]*");
    int pos = 0;
    QStringList ids = QStringList();
    QList< QPair<int,int> > positions = QList< QPair<int,int> >();
    QList <int> ids_pos = QList <int>();
    while ((pos = formula.indexOf(pattern, pos)) != -1)
    {
        QString id = formula.mid(pos, pattern.matchedLength());
        ids.append(id);
        ids_pos.append(pos);
        positions.append(getLocation(id));
        pos += pattern.matchedLength();
    }

    QMapIterator<int,int> it(selection);
    it.next();
    int firstRow = it.key();
    int firstColumn = it.value();
    setFormula(firstRow, firstColumn, formula);
    while (it.hasNext())
    {
        it.next();  
        QString result = formula;
        for (int i=0; i<ids.length(); i++)
        {
            QPair<int,int> position = positions.at(i);
            QString new_id = getLocation(position.first+it.key()-firstRow, 
                                         position.second+it.value()-firstColumn);
            if (!new_id.isEmpty())
                result.replace(ids_pos.at(i), ids.at(i).length(),
                               new_id);
        }
        setFormula(it.key(), it.value(), result);
    }
}

void SpreadSheet::setFormula(const QString &table, 
                             const QMultiMap<int, int> &from, 
                             const QMultiMap<int, int> &to)
{
    if (!selectionsEquals(from, to))
        return;
    
    QMapIterator<int,int> fromIt(from), toIt(to);
    while (fromIt.hasNext() && toIt.hasNext())
    {
        fromIt.next();
        toIt.next();
        
        QString link = QString("=%1:%2").arg(table).
                arg(getLocation(fromIt.key(), fromIt.value()));
        setFormula(toIt.key(), toIt.value(), link);
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
    
    QVariant aux = item->data(Qt::FontRole);
    if (aux.canConvert<QFont>())
        return aux.value<QFont>();
    else
        return QApplication::font();
}

QString SpreadSheet::getLinkData(const QString &formula,
                    const QHash<QString,QString> &matches) const
{
    if (mng)
    {
        QHash<QString,QString> values = mng->getLinkData(matches);
        QString result = formula;
        QHashIterator<QString,QString> it(values);
        while (it.hasNext())
        {
            it.next();
            result.replace(it.key(), it.value());
        }
        QString val;
        QFont f;
        QBrush b;
        QByteArray cellData = QCA::hexToArray(result);
        QDataStream in(&cellData, QIODevice::ReadOnly);
        in >> f >> b >> b >> val;
        return val;
    }
    else
        return "#####";
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
    if (row >= rowCount() || column >= columnCount())
        return;
    
    Cell *c = cell(row, column);
    if (!c)
    {
        c = new Cell();
        setItem(row, column, c);
    }
    connect(c, SIGNAL(invalidFormula(QString)),
            this, SIGNAL(invalidFormula(QString)));
    c->setData(Qt::EditRole, formula);
    somethingChanged(c);
}

void SpreadSheet::cut()
{
    copy();
    del();
}

void SpreadSheet::copy()
{
    QMultiMap<int,int> range = selectedItemIndexes();
    QList<int> rows = range.uniqueKeys();
    QString str = "";
    
    QList<int> cols = range.values();
    qSort(cols);
    int firstCol = cols.first(), lastCol = cols.last();
    
    for (int i=rows.first(); i<=rows.last(); i++)
    {
        if (rows.contains(i))
        {
            QList<int> columns = range.values(i);
            qSort(columns);
            
            for (int j=firstCol; j<=lastCol; j++)
            {
                if (columns.contains(j))
                    str += formula(i, j);
                str += "\t";
            }
            str.chop(1);
        }
        str += "\n";
    }
    str.chop(1);
    QApplication::clipboard()->setText(str);
}

void SpreadSheet::paste()
{
    QMultiMap<int,int> range = selectedItemIndexes();
    QMapIterator<int,int> it(range);
    it.next();
    int firstRow = it.key(), firstCol = it.value();
    
    QString str = QApplication::clipboard()->text();
    QStringList rows = str.split('\n');
    int numRows = rows.size();
    for (int r=0; r<numRows; r++)
    {
        QStringList columnData = rows.at(r).split('\t');
        int numColumns = columnData.size();
        for (int c=0; c<numColumns; c++)
        {
            int destRow = firstRow+r, destCol = firstCol+c;
            if (range.contains(destRow, destCol))
                setFormula(destRow, destCol, columnData.at(c));
        }
    }
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

void SpreadSheet::setSelectedItemIndexes(const QMultiMap<int, int> &items)
{
    clearSelection();
    QMapIterator<int, int> it(items);
    while (it.hasNext())
    {
        it.next();
        Cell *c = cell(it.key(), it.value());
        if (!c)
        {
            c = new Cell();
            setItem(it.key(), it.value(), c);
        }
        c->setSelected(true);
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

void SpreadSheet::emitSelectionChanged()
{
    emit itemSelectionChanged(selectedItemIndexes());
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
                    arg(row(cell)).
                    arg(column(cell)).
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
                    
            Cell *item = cell(it.key(), c);
            if (!item)
            {
                item = new Cell();
                setItem(it.key(), c, item);
            }
            item->setData(Qt::EditRole, formula);
            item->setFont(font);
            item->setForeground(foreground);
            item->setBackground(background);
        }
    }
}

void SpreadSheet::currentSelectionChanged()
{
    QFont f = QApplication::font();
    QBrush back = QBrush(Qt::white);
    QBrush fore = QBrush(Qt::black);
    
    QTableWidgetItem *item = currentItem();
    if (item)
    {
        QVariant aux = item->data(Qt::FontRole);
        if (aux.canConvert<QFont>())
            f = aux.value<QFont>();
        
        aux = item->data(Qt::BackgroundRole);
        if (aux.canConvert<QBrush>())
            back = aux.value<QBrush>();
        if (!back.color().isValid())
            back = QBrush(Qt::white);
        
        aux = item->data(Qt::ForegroundRole);
        if (aux.canConvert<QBrush>())
            fore = aux.value<QBrush>();
        if (!fore.color().isValid())
            fore = QBrush(Qt::black);
    }

    emit currentSelectionChanged(f, back, fore);
}

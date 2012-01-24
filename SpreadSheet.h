#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include <QtCore>
#include <QtGui>
#include <QtCrypto>
#include "DBManager.h"

class Cell;

class SpreadSheet : public QTableWidget
{
    Q_OBJECT
public:
    SpreadSheet(int rows = 100, int columns = 6, QWidget *parent = 0,
                const DBManager *const mng = 0);
    ~SpreadSheet();

    bool printSpreadSheet(const QString &fileName) const;

    QString currentLocation() const;
    QString getLocation(int row, int column) const;
    QString currentFormula() const;
    static QPair<int,int> getLocation(const QString &cellId);
    static bool selectionsEquals(const QMultiMap<int,int> &first,
                                const QMultiMap<int,int> &second);
    QMultiMap<int,int> selectedItemIndexes() const;
    QTimer *getTimer() const;
    void setRefreshTime(int sec) const;
    void replaceTimestamp(int index, const QString &newVal) const;
    void addTimestamp(int index, const QString &ts) const;
    int timestampCount() const;
    QString getTimestamp(int index) const;
    void setCellsSize(const QSize &size);
    QList<int> selectedColumns();
    QString headerText(int column);
    void setHeaderText(int column, const QString &text);
    QFont currentFont() const;
    QString getLinkData(const QString &formula,
                        const QHash<QString,QString> &matches) const;

private:
    QTimer *refresh_timer;
    void clear();
    Cell* cell(int row,int column) const;
    QString text(int row, int column) const;
    QString formula(int row, int column) const;
    void setFormula(int row, int column, const QString &formula);
    mutable QMap<int,QString> *timestamps;
    const DBManager *mng;

signals:
    void modified(const QString &cellData);
    void invalidFormula(const QString &message) const;
    void columnResize(int logicalIndex, int oldSize, int newSize);
    void rowResize(int logicalIndex, int oldSize, int newSize);
    void columnHeaderTextChanged(int column, const QString &text);
    void currentSelectionChanged(const QFont &font, 
                                 const QBrush &background,
                                 const QBrush &foreground);
    void itemSelectionChanged(const QMultiMap<int,int> &selection);

public slots:
    void setFormula(const QString &formula,
                    const QMultiMap<int,int> &selection);
    void setFormula(const QString &table,
                    const QMultiMap<int,int> &from,
                    const QMultiMap<int,int> &to);
    void cut();
    void copy();
    void paste();
    void del();
    void setSelectedItemIndexes(const QMultiMap<int,int> &items);
    void setCurrentColumnHeaderText(const QString &text);
    void setCurrentCellsFont(const QFont &f);
    void setFontColor(const QColor &c);
    void setBackgroundColor(const QColor &c);
    void setRowsSize(const QMap<int,int> size);
    void setColumnsSize(const QMap<int,int> size);
    void setColumnsHeaderText(const QMap<int, QString> data);
    void selectCurrentRow();
    void selectCurrentColumn();
    void addColumns(int columns);
    void addRows(int rows);
    void removeColumns(const QList <int> column_ids);
    void setRights(const QList<int> columns);
    void setSize(int rows, int columns);
    void emitSelectionChanged();

private slots:
    void somethingChanged(QTableWidgetItem *cell);
    void loadData(const QMap<int, QString> &data);
    void currentSelectionChanged();
};

#endif // SPREADSHEET_H

#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include "Cell.h"
#include <QtCrypto>

class SpreadSheet : public QTableWidget
{
    Q_OBJECT
public:
    SpreadSheet(int rows = 100, int columns = 6, QWidget *parent = 0);
    ~SpreadSheet();

    bool printSpreadSheet(const QString &fileName) const;

    QString currentLocation() const;
    QString getLocation(int row, int column) const;
    QString currentFormula() const;
    QTableWidgetSelectionRange selectedRange() const;
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

private:
    QTimer *refresh_timer;
    void clear();
    Cell* cell(int row,int column) const;
    QString text(int row, int column) const;
    QString formula(int row, int column) const;
    mutable QMap<int,QString> *timestamps;

signals:
    void modified(const QString &cellData);
    void getLink(const QString &table, int row,
                 int column, int destRow, int destCol) const;
    void invalidFormula(const QString &message) const;
    void columnResize(int logicalIndex, int oldSize, int newSize);
    void rowResize(int logicalIndex, int oldSize, int newSize);
    void columnHeaderTextChanged(int column, const QString &text);
    void currentSelectionChanged(const QFont &font, 
                                 const QBrush &background,
                                 const QBrush &foreground);

public slots:
    void setFormula(const QString &formula);
    void setFormula(int row, int column, const QString &formula);
    void cut();
    void copy();
    void paste();
    void del();
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

private slots:
    void somethingChanged(QTableWidgetItem *cell);
    void loadData(const QMap<int, QString> &data);
    void emitGetLink(const QString &table, int r,
                     int c, const QTableWidgetItem* cell);
    void currentSelectionChanged();
};

#endif // SPREADSHEET_H

#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include "Cell.h"

class SpreadSheet : public QTableWidget
{
    Q_OBJECT
public:
    SpreadSheet(int rows = 600, int columns = 6, QWidget *parent = 0);
    ~SpreadSheet();

    bool printSpreadSheet(const QString &fileName) const;

    QString currentLocation() const;
    QString getLocation(int row, int column) const;
    QString currentFormula() const;
    QTableWidgetSelectionRange selectedRange() const;
    QTimer *getTimer() const;
    int getNonzeroRowCount(int column) const;
    int getMaxNonzeroRowCount() const;
    void replaceTimestamp(int index, const QString &newVal) const;
    void addTimestamp(int index, const QString &ts) const;
    int timestampCount() const;
    QString getTimestamp(int index) const;
    void setCellsSize(const QSize &size);
    void setFormula(const QString &formula);
    QList<int> selectedColumns();

private:
    QTimer *refresh_timer;
    void clear();
    Cell* cell(int row,int column) const;
    QString text(int row, int column) const;
    QString formula(int row, int column) const;
    //mutable QStringList *timestamps;
    mutable QMap<int,QString> *timestamps;

signals:
    void modified(const QString &cellData);
    void getLink(const QString &table, int row,
                 int column, int destRow, int destCol) const;
    void invalidFormula(const QString &message) const;
    void columnResize(int logicalIndex, int oldSize, int newSize);
    void rowResize(int logicalIndex, int oldSize, int newSize);

public slots:
    void setFormula(int row, int column, const QString &formula);
    void cut();
    void copy();
    void paste();
    void del();
    void setCurrentColumnHeaderText(const QString &text);
    void setRowsSize(const QMap<int,int> size);
    void setColumnsSize(const QMap<int,int> size);
    void selectCurrentRow();
    void selectCurrentColumn();
    void findNext(const QString &str, Qt::CaseSensitivity cs);
    void findPrevious(const QString &str, Qt::CaseSensitivity cs);
    void addColumns(int columns);
    void addRows(int rows);
    void removeColumns(const QList <int> column_ids);
    void setRights(const QList<int> columns);

private slots:
    void somethingChanged(QTableWidgetItem *cell);
    void loadData(const QStringList &data);
    void emitGetLink(const QString &table, int r,
                     int c, const QTableWidgetItem* cell);
};

#endif // SPREADSHEET_H

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
    void replaceTimestamp(int index, const QString &newVal) const;
    void addTimestamp(const QString &ts) const;
    int timestampCount() const;
    QString getTimestamp(int index) const;
    void setCellsSize(const QSize &size);
    void setFormula(const QString &formula);

private:
    QTimer *refresh_timer;
    void clear();
    Cell* cell(int row,int column) const;
    QString text(int row, int column) const;
    QString formula(int row, int column) const;
    mutable QStringList *timestamps;

signals:
    void modified(const QString &cellData);
    void getLink(const QString &table, int row, int column) const;

public slots:
    void setFormula(int row, int column, const QString &formula);
    void cut();
    void copy();
    void paste();
    void del();
    void selectCurrentRow();
    void selectCurrentColumn();
    void findNext(const QString &str, Qt::CaseSensitivity cs);
    void findPrevious(const QString &str, Qt::CaseSensitivity cs);
    void addColumns(int columns);
    void addRows(int rows);

private slots:
    void somethingChanged(QTableWidgetItem *cell);
    void loadData(const QStringList &data);
};

#endif // SPREADSHEET_H

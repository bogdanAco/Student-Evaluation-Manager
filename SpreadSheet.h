#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include "Cell.h"

class SpreadSheet : public QTableWidget
{
    Q_OBJECT
public:
    SpreadSheet(int rows = 600, int columns = 6, QWidget *parent = 0);
    ~SpreadSheet();

    bool printSpreadSheet(const QString &fileName);

    QString currentLocation() const;
    QString getLocation(int row, int column) const;
    QString currentFormula() const;
    QTableWidgetSelectionRange selectedRange() const;
    QTimer *getTimer() const;
    int getNonzeroRowCount(int column);
    void replaceTimestamp(int index, const QString &newVal);
    void addTimestamp(const QString &ts);
    int timestampCount();
    QString getTimestamp(int index) const;
    void setCellsSize(const QSize &size);
    void setFormula(const QString &formula);

private:
    QTimer *refresh_timer;
    void clear();
    Cell* cell(int row,int column) const;
    QString text(int row, int column) const;
    QString formula(int row, int column) const;
    void setFormula(int row, int column, const QString &formula);
    QStringList *timestamps;

signals:
    void modified(const QString &cellData);

public slots:
    void cut();
    void copy();
    void paste();
    void del();
    void selectCurrentRow();
    void selectCurrentColumn();
    void findNext(const QString &str, Qt::CaseSensitivity cs);
    void findPrevious(const QString &str, Qt::CaseSensitivity cs);

private slots:
    void somethingChanged(QTableWidgetItem *cell);
    void loadData(const QStringList &data);
};

#endif // SPREADSHEET_H

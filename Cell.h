#ifndef CELL_H
#define CELL_H

#include <QtCore>
#include <QtGui>
#include "SpreadSheet.h"

class Cell : public QObject, public QTableWidgetItem
{
    Q_OBJECT
public:
    Cell();
    Cell(const QString &text);

    QVariant data(int role) const;
    void setData(int role, const QVariant &value);
    QVariant display(const QString &data = QString()) const;
    QString formula() const;
    QVariant computeFormula(const QString &formula,
                            SpreadSheet *widget) const;
private:
    bool isValidFormula(const QString &formula) const;
    QVariant parseMember(const QString &formula,
                         SpreadSheet *widget) const;
    QString compareMembers(int &param_no, const QString &op,
                        double &firstOperand, double &secondOperand,
                        const QString &case1, const QString &case2) const;
    bool compareMembers(const QString &op, double &firstOperand, double &secondOperand) const;
    QVariant getCellValue(const QString &id, SpreadSheet *widget) const;
    int firstOperatorPosition(const QString &formula) const;

signals:
    void invalidFormula(const QString &message) const;
};

#endif // CELL_H

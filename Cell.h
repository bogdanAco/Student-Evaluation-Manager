#ifndef CELL_H
#define CELL_H

#include <QtGui>

class Cell : public QTableWidgetItem
{
public:
    Cell();
    Cell(const QString &text);

    QTableWidgetItem* clone() const;

    QVariant data(int role) const;
    void setData(int role, const QVariant &value);
    QVariant display() const;

    inline QString formula() const
         { return QTableWidgetItem::data(Qt::DisplayRole).toString(); }
    QVariant computeFormula(const QString &formula,
                            const QTableWidget *widget) const;
private:
    bool isValidFormula(const QString &formula) const;
    QVariant parseMember(const QString &formula,
                         const QTableWidget *widget) const;
    QString compareMembers(int &param_no, int &op_len, const QString &op,
                        double &firstOperand, double &secondOperand,
                        const QString &case1, const QString &case2) const;
    bool compareMembers(int &op_len, const QString &op,
                        double &firstOperand, double &secondOperand) const;
    QVariant getCellValue(const QString &id,
                  const QTableWidget *widget) const;
    int firstOperatorPosition(const QString &formula) const;
};

#endif // CELL_H

#include "Cell.h"
#include <QRegExp>

Cell::Cell() : QTableWidgetItem() { }

Cell::Cell(const QString &text) : QTableWidgetItem(text) { }

QTableWidgetItem* Cell::clone() const
{
    Cell *item = new Cell();
    *item = *this;
    return item;
}

QVariant Cell::data(int role) const
{
    if (role == Qt::EditRole || role == Qt::StatusTipRole)
        return formula();
    else if (role == Qt::DisplayRole)
        return display();
    else if (role == Qt::TextAlignmentRole)
        return int(Qt::AlignRight | Qt::AlignVCenter);
    else
        return QTableWidgetItem::data(role);
}

void Cell::setData(int role, const QVariant &value)
{
    QTableWidgetItem::setData(role, value);
    if (tableWidget())
        tableWidget()->viewport()->update();
}

QVariant Cell::display() const
{
    QString temp = formula();
    if (!isValidFormula(temp))
        return temp;

    QString aux = "((";
    for (int i=1; i<temp.length(); i++)
        if (temp[i] == '+')
            aux.append("))+((");
        else if (temp[i] == '-')
            aux.append("))-((");
        else if (temp[i] == '*')
            aux.append(")*(");
        else if (temp[i] == '/')
            aux.append(")/(");
        else if (temp[i] == '(')
            aux.append("(((");
        else if (temp[i] == ')')
            aux.append(")))");
        else
            aux.append(temp[i]);
    aux.append("))");

    if (!isValidFormula("=" + aux))
        return temp;
    QVariant result = computeFormula(aux, tableWidget());
    return result;
}

QVariant Cell::computeFormula(const QString &formula, const QTableWidget *widget) const
{
    QVariant result = QVariant::Invalid,
             firstOperand = QVariant::Invalid,
             secondOperand = QVariant::Invalid;
    QString aux = formula;
    int firstOp = firstOperatorPosition(aux);
    if (firstOp == -1)
        result = parseMember(aux, widget);
    while (firstOp != -1)
    {
        if (result == QVariant::Invalid)
            firstOperand = parseMember(aux.left(firstOp), widget);
        else
            firstOperand = result;
        QChar firstOperator = aux[firstOp];
        aux = aux.mid(firstOp+1);
        firstOp = firstOperatorPosition(aux);
        if (firstOp == -1)
            secondOperand = parseMember(aux, widget);
        else
            secondOperand = parseMember(aux.left(firstOp), widget);

        switch (firstOperator.toAscii())
        {
            case '+':
                result = firstOperand.toDouble() + secondOperand.toDouble();
                break;
            case '-':
                result = firstOperand.toDouble() - secondOperand.toDouble();
                break;
            case '*':
                result = firstOperand.toDouble() * secondOperand.toDouble();
                break;
            case '/':
                result = firstOperand.toDouble() / secondOperand.toDouble();
                break;
        }
    }
    return result;
}

bool Cell::isValidFormula(const QString &formula) const
{
    if (formula == "")
        return false;
    if (formula.at(0) != '=')
        return false;
    else if (formula.count('(') != formula.count(')'))
        return false;
    else
        return true;
}

QVariant Cell::parseMember(const QString &formula, const QTableWidget *widget) const
{
    int fOp = firstOperatorPosition(formula);
    if (fOp == -1)
        fOp = formula.length();
    QChar first = formula.at(0);
    //paranteza
    if (first=='(')
    {
        int end = 0;
        int open_p_count = 0;
        int closed_p_count = 0;
        for (int c=0; c<formula.length(); c++)
        {
            if (formula.at(c) == '(')
                open_p_count++;
            else if (formula.at(c) == ')')
                closed_p_count++;
            if (open_p_count == closed_p_count)
            {
                end = c;
                break;
            }
        }
        return computeFormula(formula.mid(1,end-1), widget);
    }
    //numar 0 sau 0.0
    else if (first.isDigit())
    {
        QString s = formula.left(fOp);
        if (s.count('.') <= 1)
        {
            bool ok;
            double x = s.toDouble(&ok);
            if (ok)
                return x;
            else
                return "#####";
        }
        else
            return "#####";
    }
    //celula A2
    else if (first.isUpper())
    {
        return getCellValue(formula,widget);
    }
    //functie sum(A1;A2;A3)
    else if (first.isLower())
    {
        QStringList simple_function_names;
        QStringList cond_function_names;
        QStringList parameters;
        simple_function_names << "sum" << "avg" << "count";
        cond_function_names << "if" << "countif";
        QString s = formula.left(fOp);

        QString params = s.mid(s.lastIndexOf('(')+1,
                               s.indexOf(')')-s.lastIndexOf('(')-1);
        if (s.count('(') == s.count(')'))
            parameters = params.split(';');
        else
            return "#####";
        s = formula.left(formula.indexOf('('));
        if (simple_function_names.contains(s))
        {
            QVariantList values;
            QRegExp pattern("^([A-Z][0-9]{1,3})$");
            foreach (QString str, parameters)
                if (pattern.exactMatch(str))
                    values.append(getCellValue(str,widget));
                else
                    return "#####";

            if (s == "sum")
            {
                double tmp = 0;
                bool ok = true;
                foreach (QVariant aux, values)
                {
                    tmp += aux.toDouble(&ok);
                    if (!ok)
                        return "#####";
                }
                return tmp;
            }
            else if (s == "avg")
            {
                double tmp = 0;
                bool ok = true;
                foreach (QVariant aux, values)
                {
                    tmp += aux.toDouble(&ok);
                    if (!ok)
                        return "#####";
                }
                tmp /= parameters.length();
                return tmp;
            }
            else if (s == "count")
            {
                int length = 0;
                foreach (QString aux, parameters)
                    if (getCellValue(aux,widget) != "")
                        length++;
                return length;
            }
        }
        else if (cond_function_names.contains(s))
        {
            int param_no = parameters.length();
            if (s == "if")
            {
                //if(A1<5;"Picat";n)
                //if(A1<5;4)

                QRegExp pattern("^([A-Z]\\d{1,3}(<|<=|>|>=|<>|=)(([A-Z]\\d{1,3})|(\\d+(\\.\\d+)?)))$");
                if (pattern.exactMatch(parameters[0]))
                {
                    QString condition = parameters[0];
                    int length = 1;
                    int opPos = condition.indexOf(QRegExp("(<|>|=)"));
                    if (condition.indexOf(QRegExp("(<=|>=|<>)")) > -1)
                        length = 2;
                    QString op = condition.mid(opPos,length);
                    double firstOperand = parseMember(condition.left(opPos), widget).toDouble();
                    double secondOperand = parseMember(condition.mid(opPos+length), widget).toDouble();
                    if (param_no == 2)
                    {
                        QString empty_str = "";
                        return compareMembers(param_no, length, op,
                                              firstOperand, secondOperand,
                                              parameters[1], empty_str);
                    }
                    else if (param_no == 3)
                        return compareMembers(param_no, length, op,
                                              firstOperand, secondOperand,
                                              parameters[1], parameters[2]);
                    else
                        return "#####";
                }
                else
                    return "#####";
            }
            else if (s == "countif")
            {
                //countif(A1;A2...An;>5)
                if (param_no > 1)
                {
                    int count = 0;
                    int length = 1;
                    QString condition = parameters[param_no-1];
                    int opPos = condition.indexOf(QRegExp("(<|>|=)"));
                    if (condition.indexOf(QRegExp("(<=|>=|<>)")) > -1)
                        length = 2;
                    QString op = condition.mid(opPos,length);
                    double firstOperand;
                    double secondOperand = parseMember(condition.mid(opPos+length), widget).toDouble();
                    QRegExp pattern("^([A-Z][0-9]{1,3})$");
                    for (int i=0; i<param_no-1; i++)
                        if (pattern.exactMatch(parameters[i]))
                        {
                            firstOperand = getCellValue(parameters[i], widget).toDouble();
                            if (compareMembers(length, op, firstOperand, secondOperand))
                                count++;
                        }
                    return count;
                }
            }
            else
                return "#####";
        }
        else
            return "#####";
    }
    else
        return "#####";
    return "#####";
}

QString Cell::compareMembers(int &param_no, int &op_len, const QString &op,
                    double &firstOperand, double &secondOperand,
                    const QString &case1, const QString &case2) const
{
    if (op == "<" || op == "<=")
    {
        if (op_len == 1 && firstOperand < secondOperand)
            return case1;
        else if (op_len == 2 && firstOperand <= secondOperand)
            return case1;
        else if (param_no == 3)
            return case2;
    }
    else if (op == ">" || op == ">=")
    {
        if (op_len == 1 && firstOperand > secondOperand)
            return case1;
        else if (op_len == 2 && firstOperand >= secondOperand)
            return case1;
        else if (param_no == 3)
            return case2;
    }
    else if (op == "<>")
    {
        if (firstOperand != secondOperand)
            return case1;
        else if (param_no == 3)
            return case2;
    }
    else if (op == "=")
    {
        if (firstOperand == secondOperand)
            return case1;
        else if (param_no == 3)
            return case2;
    }
    return "#####";
}

bool Cell::compareMembers(int &op_len, const QString &op,
                    double &firstOperand, double &secondOperand) const
{
    if (op == "<" || op == "<=")
    {
        if (op_len == 1 && firstOperand < secondOperand)
            return true;
        else if (op_len == 2 && firstOperand <= secondOperand)
            return true;
        return false;
    }
    else if (op == ">" || op == ">=")
    {
        if (op_len == 1 && firstOperand > secondOperand)
            return true;
        else if (op_len == 2 && firstOperand >= secondOperand)
            return true;
        return false;
    }
    else if (op == "<>")
    {
        if (firstOperand != secondOperand)
            return true;
        return false;
    }
    else if (op == "=")
    {
        if (firstOperand == secondOperand)
            return true;
        return false;
    }
    return false;
}

QVariant Cell::getCellValue(const QString &id,
              const QTableWidget *widget) const
{
    int column = ((int)(id[0].toAscii()))-65;
    QString s = id.mid(1,firstOperatorPosition(id));
    bool ok;
    int row = (s.toInt(&ok))-1;
    if (ok
        && row >= 0
        && row < widget->height()
        && column >= 0
        && column < widget->width()
        )
    {
        const QTableWidgetItem* value = widget->item(row,column);
        if (value == 0)
            return "";
        return value->data(Qt::DisplayRole);
    }
    else
        return "#####";
}

int Cell::firstOperatorPosition(const QString &formula) const
{
    char operators[] = {'+','-','*','/'};
    for (int i=0; i<formula.length(); i++)
    {
        if (formula[i] == '(')
        {
            int end = i;
            int open_p_count = 0;
            int closed_p_count = 0;
            for (int c=i; c<formula.length(); c++)
            {
                if (formula.at(c) == '(')
                    open_p_count++;
                else if (formula.at(c) == ')')
                    closed_p_count++;
                if (open_p_count == closed_p_count)
                {
                    end = c;
                    break;
                }
            }
            i = end;
        }
        else
            for (int j=0; j<4; j++)
                if (formula[i] == operators[j])
                    return i;
    }
    return -1;
}

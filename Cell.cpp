#include "Cell.h"
#include <QRegExp>

Cell::Cell() : QTableWidgetItem() { }

Cell::Cell(const QString &text) : QTableWidgetItem(text) { }

QVariant Cell::data(int role) const
{
    if (role == Qt::EditRole)
        return formula();
    else if (role == Qt::DisplayRole)
            return display();
    else if (role == Qt::TextAlignmentRole)
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    else
        return QTableWidgetItem::data(role);
}

void Cell::setData(int role, const QVariant &value)
{
    QTableWidgetItem::setData(role, value);
    if (tableWidget())
        tableWidget()->viewport()->update();
}

QVariant Cell::display(const QString &data) const
{
    QString temp = (data.isEmpty())?formula():data;
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
    QVariant result = computeFormula(aux, (SpreadSheet*)tableWidget());
    return result;
}

QString Cell::formula() const
{
    return QTableWidgetItem::data(Qt::DisplayRole).toString();
}

QVariant Cell::computeFormula(const QString &formula, SpreadSheet *widget) const
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
    if (formula.isEmpty())
        return false;
    if (formula.at(0) != '=')
        return false;
    if (formula.count('(') != formula.count(')'))
        return false;
    return true;
}

QVariant Cell::parseMember(const QString &formula, SpreadSheet *widget) const
{
    int fOp = firstOperatorPosition(formula);
    if (fOp == -1)
        fOp = formula.length();
    QChar first = formula.at(0);
    
    QRegExp importedData("([\\w\\s]+:[A-Z][1-9][0-9]*)");
    QRegExp cellId("([A-Z][1-9][0-9]*)");
    
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
        }
        emit invalidFormula(QString("Invalid number or number format in %1").arg(s));
        return "#####";
    }
    //tabela:identificator
    else if (formula.indexOf(importedData) == 0)
    {
        int idx = 0;
        QHash<QString,QString> matches = QHash<QString,QString>();
        while ((idx = formula.indexOf(importedData, idx)) != -1)
        {
            QString match = formula.mid(idx, importedData.matchedLength());
            int delim = match.indexOf(':');
            QString table = match.left(delim);
            QString id = match.mid(delim+1);
            matches.insertMulti(table, id);
            idx += importedData.matchedLength();
        }
        QString result = widget->getLinkData(formula, matches);
        if (isValidFormula(result))
            return display(result);
        else
            return result;
    }
    //celula A2
    else if (cellId.exactMatch(formula))
    {
        QVariant cellVal = getCellValue(formula,widget);
        if (cellVal == "#####")
            emit invalidFormula(QString("Invalid cell data in %1").arg(formula));
        return cellVal;
    }
    //functie nume_functie(A1;A2;A3)
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
        {
            emit invalidFormula(QString("Invalid paranthesis number ").append(s));
            return "#####";
        }
        s = formula.left(formula.indexOf('('));
        if (simple_function_names.contains(s))
        {
            QVariantList values;
            QListIterator<QString> it(parameters);
            while (it.hasNext())
            {
                QString str = it.next();
                QVariant val = parseMember(str, widget);
                if (val != "#####")
                    values.append(val);
            }

            if (s == "sum")
            {
                double tmp = 0;
                bool ok = true;
                QListIterator<QVariant> valIt(values);
                while (valIt.hasNext())
                {
                    QVariant aux = valIt.next();
                    tmp += aux.toDouble(&ok);
                    if (!ok)
                    {
                        emit invalidFormula(QString("Not a number: ").append(aux.toString()));
                        return "#####";
                    }
                }
                return tmp;
            }
            else if (s == "avg")
            {
                double tmp = 0;
                bool ok = true;
                QListIterator<QVariant> valIt(values);
                while (valIt.hasNext())
                {
                    QVariant aux = valIt.next();
                    tmp += aux.toDouble(&ok);
                    if (!ok)
                    {
                        emit invalidFormula(QString("Not a number: ").append(aux.toString()));
                        return "#####";
                    }
                }
                tmp /= parameters.length();
                return tmp;
            }
            else if (s == "count")
            {
                return values.length();
            }
        }
        else if (cond_function_names.contains(s))
        {
            int param_no = parameters.length();
            if (param_no < 2)
            {
                emit invalidFormula(QString("Invalid parameter number: %1").arg(param_no));
                return "#####";
            }
            
            if (s == "if")
            {
                //if(A1<5;"Picat";n)
                //if(A1<5;4)
                QRegExp pattern("^(([\\w\\s]+:)?[A-Z][1-9][0-9]*"
                                "(<|<=|>|>=|<>|=)"
                                "((([\\w\\s]+:)?[A-Z][1-9][0-9]*)|(\\d+(\\.\\d+)?)))$");
                QString condition = parameters.at(0);
                if (pattern.exactMatch(condition) && param_no <= 3)
                {
                    int length = 1;
                    int opPos = condition.indexOf(QRegExp("(<|>|=)"));
                    if (condition.indexOf(QRegExp("(<=|>=|<>)")) > -1)
                        length = 2;
                    QString op = condition.mid(opPos,length);
                    bool ok1, ok2;
                    double firstOperand = parseMember(condition.left(opPos), widget).toDouble(&ok1);
                    double secondOperand = parseMember(condition.mid(opPos+length), widget).toDouble(&ok2);
                    if (!ok1 || !ok2)
                    {
                        emit invalidFormula(QString("Invalid condition parameters: %1").arg(condition));
                        return "#####";
                    }
                    if (param_no == 2)
                        return compareMembers(param_no, op,
                                              firstOperand, secondOperand,
                                              parameters.at(1), "#####");
                    else if (param_no == 3)
                        return compareMembers(param_no, op,
                                              firstOperand, secondOperand,
                                              parameters.at(1), parameters.at(2));
                }
                else
                {
                    emit invalidFormula(QString("Invalid formula syntax: ").append(condition));
                    return "#####";
                }
            }
            else if (s == "countif")
            {
                //countif(A1;A2...An;>5)
                if (param_no > 2)
                {
                    int count = 0;
                    int length = 1;
                    QString condition = parameters.last();
                    int opPos = condition.indexOf(QRegExp("(<|>|=)"));
                    if (condition.indexOf(QRegExp("(<=|>=|<>)")) > -1)
                        length = 2;
                    if (opPos == -1)
                    {
                        emit invalidFormula(QString("Invalid condition syntax: ").append(condition));
                        return "#####";
                    } 
                    QString op = condition.mid(opPos,length);
                    bool ok;
                    double firstOperand;
                    double secondOperand = parseMember(condition.mid(opPos+length), widget).toDouble(&ok);
                    if (!ok)
                    {
                        emit invalidFormula(QString("Invalid second operand: %1").
                                            arg(condition.mid(opPos+length)));
                        return "#####";
                    }
                    for (int i=0; i<param_no-1; i++)
                    {
                        firstOperand = parseMember(parameters.at(i), widget).toDouble(&ok);
                        if (!ok)
                        {
                            emit invalidFormula(QString("Invalid operand: %1").
                                                arg(parameters.at(i)));
                            return "#####";
                        }
                        if (compareMembers(op, firstOperand, secondOperand))
                            count++;
                    }
                    return count;
                }
            }
        }
        else
        {
            emit invalidFormula("Invalid formula");
            return "#####";
        }
    }
    return formula;
}

QString Cell::compareMembers(int &param_no, const QString &op,
                    double &firstOperand, double &secondOperand,
                    const QString &case1, const QString &case2) const
{
    if (op.size() == 1)
    {
        if (op == "<")
        {
            if (firstOperand < secondOperand)
                return case1;
            else
                return case2;
        }
        else if (op == ">")
        {
            if (firstOperand > secondOperand)
                return case1;
            else
                return case2;
        }
        else if (op == "=")
        {
            if (firstOperand == secondOperand)
                return case1;
            else
                return case2;
        }
    }
    else if (op.size() == 2)
    {
        if (op == "<=")
        {
            if (firstOperand <= secondOperand)
                return case1;
            else
                return case2;
        }
        else if (op == ">=")
        {
            if (firstOperand >= secondOperand)
                return case1;
            else
                return case2;
        }
        else if (op == "<>")
        {
            if (firstOperand != secondOperand)
                return case1;
            else
                return case2;
        }
    }
    emit invalidFormula(QString("Invalid operator: ").append(op));
    return "#####";
}

bool Cell::compareMembers(const QString &op, double &firstOperand, double &secondOperand) const
{
    if (op.size() == 1)
    {
        if (op == "<")
        {
            if (firstOperand < secondOperand)
                return true;
        }
        else if (op == ">")
        {
            if (firstOperand > secondOperand)
                return true;
        }
        else if (op == "=")
        {
            if (firstOperand == secondOperand)
                return true;
        }
        return false;
    }
    else if (op.size() == 2)
    {
        if (op == "<=")
        {
            if (firstOperand <= secondOperand)
                return true;
        }
        else if (op == ">=")
        {
            if (firstOperand >= secondOperand)
                return true;
        }
        else if (op == "<>")
        {
            if (firstOperand != secondOperand)
                return true;
        }
        return false;
    }
    else
        return false;
}

QVariant Cell::getCellValue(const QString &id,
              SpreadSheet *widget) const
{
    int column = ((int)(id[0].toAscii()))-65;
    QString s = id.mid(1,firstOperatorPosition(id));
    bool ok;
    int row = (s.toInt(&ok))-1;
    if (ok
        && row >= 0
        && column >= 0
        && row < widget->rowCount()
        && column < widget->columnCount()
        )
    {
        const QTableWidgetItem* value = widget->item(row,column);
        if (value == 0)
            return "#####";
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

#include "Dialog.h"

Dialog::Dialog(const QString &title, const QString &text, QWidget* parent) :
        QDialog(parent)
{
    setMinimumWidth(200);
    mainLayout = new QGridLayout(this);
    this->text = new QLabel(text,this);
    this->text->setWordWrap(true);
    setWindowTitle(title);
    mainLayout->addWidget(this->text, 3, 0, 1, 4);

    result = new QLabel("",this);
    result->setStyleSheet("QLabel { color:red; }");
    mainLayout->addWidget(result, 15, 0, 1, 4, Qt::AlignRight);
    ok = new QPushButton(style()->standardIcon(QStyle::SP_DialogOkButton),"Ok",this);
    ok->setFixedWidth(75);
    mainLayout->addWidget(ok, 16, 2, Qt::AlignRight);
    cancel = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton),
                             "Cancel",this);
    cancel->setFixedWidth(75);
    connect(cancel,SIGNAL(clicked()),this,SLOT(close()));
    mainLayout->addWidget(cancel, 16, 3, Qt::AlignRight);

    connect(ok, SIGNAL(clicked()), this, SLOT(emitOKSignal()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(emitCancelSignal()));
}

Dialog::~Dialog()
{
    this->hide();
    delete text;
    delete result;
    delete ok;
    delete cancel;
    delete mainLayout;
}

void Dialog::emitOKSignal()
{
    emit okPressed();
}

void Dialog::emitCancelSignal()
{
    emit cancelPressed();
}

void Dialog::showMessage(const QString &msg)
{
    result->setText(msg);
}

CreateFolderDialog::CreateFolderDialog(QWidget *parent) :
        Dialog("Create folder", "Create folder", parent)
{
    result->hide();
    value = new QLineEdit();
    mainLayout->addWidget(value, 4, 0, 1, 4);
    disconnect(this);
    connect(this, SIGNAL(okPressed()),
            this, SLOT(emitSelected()));
}

CreateFolderDialog::~CreateFolderDialog()
{
    delete value;
}

void CreateFolderDialog::emitSelected()
{
    this->hide();
    emit selected(value->text());
}

ModifyDialog::ModifyDialog(const QString &type, QWidget *parent) :
        Dialog(type, QString("Modify spreadsheet:\n").append(type), parent)
{
    value = new QLineEdit("1");
    mainLayout->addWidget(value, 4, 0, 1, 4);
    connect(this, SIGNAL(okPressed()), this, SLOT(checkData()));
}

ModifyDialog::~ModifyDialog()
{
    delete value;
}

void ModifyDialog::checkData()
{
    bool ok = false;
    int count = value->text().toInt(&ok);
    if (!ok)
    {
        result->setText("Please enter a number");
        return;
    }
    if (count == 0 || count > 10)
    {
        result->setText("Number must be between 1-10");
        return;
    }
    this->hide();
    emit dataChecked(count);
}

UserLoginDialog::UserLoginDialog(QWidget *parent) :
        Dialog("User login","Username",parent)
{
    usrnm = new QLineEdit("admin", this);
    mainLayout->addWidget(usrnm, 4, 0, 1, 4);
    password = new QLabel("Password:",this);
    mainLayout->addWidget(password, 5, 0, 1, 4);
    passwd = new QLineEdit("admin", this);
    passwd->setEchoMode(QLineEdit::Password);
    mainLayout->addWidget(passwd, 6, 0, 1, 4);


    connect(this, SIGNAL(okPressed()), this, SLOT(checkData()));
}

void UserLoginDialog::showMessage(const QString &msg)
{
    result->setText(msg);
}

void UserLoginDialog::checkData()
{
    result->setText("");
    int name_len = usrnm->text().length();
    int pass_len = passwd->text().length();
    if (name_len == 0)
    {
        result->setText("No username entered");
        return;
    }
    else if (pass_len == 0)
    {
        result->setText("No password entered");
        return;
    }
    emit dataChecked(usrnm->text(), passwd->text());
}

UserLoginDialog::~UserLoginDialog()
{
    this->hide();
    delete usrnm;
    delete password;
    delete passwd;
}

ErrorDialog::ErrorDialog(const QString& text, QWidget* parent) :
        Dialog("Error", text, parent)
{
    cancel->hide();
    connect(ok, SIGNAL(clicked()), this, SLOT(close()));
}

ErrorDialog::~ErrorDialog()
{

}

FormulaDialog::FormulaDialog(int currentRow, int currentCol,
                             const SpreadSheet *spreadsheet,
                             QWidget *parent) :
        Dialog("Formula", "Select a function", parent)
{
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->setFixedSize(220, 280);
    this->spreadsheet = spreadsheet;
    this->currentRow = currentRow;
    this->currentColumn = currentCol;

    mainLayout->removeWidget(text);
    mainLayout->addWidget(text, 0, 0, 1, 4);

    formula = new QComboBox();
    formula->addItem("sum");
    formula->addItem("avg");
    formula->addItem("count");
    formula->addItem("if");
    formula->addItem("countif");
    mainLayout->addWidget(formula, 1, 0, 1, 4);

    formulaFormat = new QLabel();
    formulaFormat->setStyleSheet("QLabel { font:bold; }");
    formulaFormat->setWordWrap(true);
    mainLayout->addWidget(formulaFormat, 2, 0, 1, 4);
    formulaInfo = new QLabel();
    formulaInfo->setWordWrap(true);
    mainLayout->addWidget(formulaInfo, 3, 0, 3, 4);

    rangeText = new QLabel("Enter or select the range:");
    mainLayout->addWidget(rangeText,
                          6, 0, 1, 4);
    range = new QLineEdit();
    mainLayout->addWidget(range, 7, 0, 1, 4);

    conditionText = new QLabel("Condition");
    conditionText->setDisabled(true);
    mainLayout->addWidget(conditionText, 8, 0, 1, 4);

    condition = new QLineEdit();
    condition->setDisabled(true);
    mainLayout->addWidget(condition, 9, 0, 1, 4);

    thenText = new QLabel("Then value:");
    mainLayout->addWidget(thenText, 10, 0, 1, 4);
    thenValue = new QLineEdit();
    mainLayout->addWidget(thenValue, 11, 0, 1, 4);

    elseText = new QLabel("Else value:");
    mainLayout->addWidget(elseText, 12, 0, 1, 4);
    elseValue = new QLineEdit();
    mainLayout->addWidget(elseValue, 13, 0, 1, 4);
    hideThenElse();

    showFormulaInfo("sum");
    connect(formula, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(showFormulaInfo(QString)));
    connect(this->spreadsheet, SIGNAL(itemSelectionChanged()),
            this, SLOT(addRangeItems()));
    connect(this, SIGNAL(okPressed()), this, SLOT(generateFormula()));
    connect(this, SIGNAL(accepted()), spreadsheet, SLOT(clearSelection()));
    connect(this, SIGNAL(cancelPressed()), spreadsheet, SLOT(clearSelection()));
}

FormulaDialog::~FormulaDialog()
{
    delete formula;
    delete formulaInfo;
    delete formulaFormat;
    delete range;
    delete conditionText;
    delete condition;
    delete thenText;
    delete thenValue;
    delete elseText;
    delete elseValue;
}

void FormulaDialog::hideThenElse()
{
    thenText->hide();
    thenValue->hide();
    elseText->hide();
    elseValue->hide();
}

void FormulaDialog::showThenElse()
{
    thenText->show();
    thenValue->show();
    elseText->show();
    elseValue->show();
}

void FormulaDialog::showFormulaInfo(const QString &formula)
{
    hideThenElse();
    this->setFixedSize(220, 280);
    formulaFormat->setText("");
    formulaInfo->setText("");
    rangeText->setText("Enter or select the range:");
    condition->setDisabled(true);
    conditionText->setDisabled(true);

    if (formula == "sum")
    {
        formulaFormat->setText("sum(value1;value2;...;valueN)");
        formulaInfo->setText("Adds all the numbers in the "
                             "selected or entered range");
    }
    else if (formula == "avg")
    {
        formulaFormat->setText("avg(value1;value2;...;valueN)");
        formulaInfo->setText("Returns the average value of "
                             "the selected or entered range");
    }
    else if (formula == "count")
    {
        formulaFormat->setText("count(value1;value2;...;valueN)");
        formulaInfo->setText("Counts the number of cells within "
                             "the selected or entered range");
    }
    else if (formula == "if")
    {
        showThenElse();
        this->setFixedSize(220, 380);
        rangeText->setText("Cell:");
        formulaFormat->setText("if(condition;then;else)");
        formulaInfo->setText("If the condition is true, returns "
                             "the first entered value, "
                             "otherwise the second one.\n"
                             "Else value is optional");
        condition->setEnabled(true);
        conditionText->setEnabled(true);
    }
    else if (formula == "countif")
    {
        formulaFormat->setText("countif(value1;...;valueN;condition)");
        formulaInfo->setText("Counts the elements within the  "
                             "selected or entered range, "
                             "that satisfies the condition");
        condition->setEnabled(true);
        conditionText->setEnabled(true);
    }
}

void FormulaDialog::addRangeItems()
{
    range->setText("");

    if (formula->currentText() == "if")
    {
        range->setText(spreadsheet->currentLocation());
        return;
    }

    QTableWidgetSelectionRange selected = spreadsheet->selectedRange();
    for (int i=selected.topRow(); i<=selected.bottomRow(); i++)
        for (int j=selected.leftColumn(); j<=selected.rightColumn(); j++)
            range->setText(range->text().
                           append(spreadsheet->getLocation(i, j)).
                           append(";"));
    range->setText(range->text().remove(range->text().length()-1, 1));
}

void FormulaDialog::generateFormula()
{
    if (formula->currentText() == "")
        return;
    if (condition->isEnabled())
        if (condition->text() == "")
            return;
    if (range->text() == "")
        return;

    QString finalFormula = "=";
    finalFormula.append(formula->currentText());
    finalFormula.append("(");
    if (formula->currentText() == "if")
    {
        finalFormula.append(range->text());
        finalFormula.append(condition->text());
        finalFormula.append(";");
        finalFormula.append(thenValue->text());
        if (elseValue->text() != "")
        {
            finalFormula.append(";");
            finalFormula.append(elseValue->text());
        }
    }
    else
    {
        finalFormula.append(range->text());
        if (formula->currentText() == "countif")
        {
            finalFormula.append(";");
            finalFormula.append(condition->text());
        }
    }
    finalFormula.append(")");
    emit setFormula(currentRow, currentColumn, finalFormula);
}

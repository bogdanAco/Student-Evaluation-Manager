#ifndef DIALOG_H
#define DIALOG_H

#include <QtGui>
#include "CFGManager.h"
#include "SpreadSheet.h"

#define OPEN_FILE
#define NEW_FILE

class Dialog : public QDialog
{
    Q_OBJECT
public:
    Dialog(const QString& title, const QString& text,
           QWidget* parent = 0);
    ~Dialog();
    QString getText();

protected:
    QGridLayout *mainLayout;
    QLabel *text;
    QLabel *result;
    QPushButton *ok;
    QPushButton *cancel;

signals:
    void okPressed();
    void cancelPressed();

protected slots:
    void showMessage(const QString &msg);
};

class CreateFolderDialog : public Dialog
{
    Q_OBJECT
public:
    CreateFolderDialog(QWidget *parent = 0);
    ~CreateFolderDialog();

signals:
    void selected(const QString &val);

private slots:
    void emitSelected();

private:
    QLineEdit *value;
};

class ModifyDialog : public Dialog
{
    Q_OBJECT
public:
    ModifyDialog(const QString &type, QWidget *parent = 0);
    ~ModifyDialog();

protected:
    QLineEdit *value;

signals:
    void dataChecked(int count);

protected slots:
    virtual void checkData();
};

class TextModifyDialog : public ModifyDialog
{
    Q_OBJECT
public:
    TextModifyDialog(const QString &type, QWidget *parent = 0);
    ~TextModifyDialog();

signals:
    void dataChecked(const QString &username);

private:
    virtual void checkData();
};

class UserLoginDialog : public Dialog
{
    Q_OBJECT
public:
    UserLoginDialog(QWidget *parent = 0);
    void showMessage(const QString &msg);
    ~UserLoginDialog();

protected:
    QLineEdit *usrnm;
    QLabel *password;
    QLineEdit *passwd;

signals:
    void dataChecked(const QString &name,
                     const QString &password);
    void saveLoginDataSignal(const QString &name,
                             const QString &password);
protected slots:
    virtual void checkData();
};

class UserSignInDialog : public UserLoginDialog
{
    Q_OBJECT
public:
    UserSignInDialog(QWidget *parent = 0);
    ~UserSignInDialog();
    
private:
    QLabel *passwdVerifyLabel;
    QLineEdit *passwdVerify;

private slots:
    void checkData();
    bool strongPassword(const QString &pass);

signals:
    void dataChecked(const QString &name,
                     const QString &pass);
};

class ErrorDialog : public Dialog
{
    Q_OBJECT
public:
    ErrorDialog(const QString& text, QWidget* parent = 0);
    ~ErrorDialog();
};

class FormulaDialog : public Dialog
{
    Q_OBJECT
public:
    FormulaDialog(int currentRow, int currentCol,
                  const SpreadSheet *spreadsheet,
                  QWidget *parent = 0);
    ~FormulaDialog();

private:
    QComboBox *formula;
    QLabel *formulaInfo;
    QLabel *formulaFormat;
    QLineEdit *range;
    QLabel *rangeText;
    QLineEdit *condition;
    QLabel *conditionText;
    QLabel *thenText;
    QLineEdit *thenValue;
    QLabel *elseText;
    QLineEdit *elseValue;
    const SpreadSheet *spreadsheet;
    int currentRow;
    int currentColumn;
    void hideThenElse();
    void showThenElse();

private slots:
    void showFormulaInfo(const QString &formula);
    void addRangeItems();
    void generateFormula();

signals:
    void setFormula(int row, int col,
                    const QString &formula);
};

#endif // DIALOG_H

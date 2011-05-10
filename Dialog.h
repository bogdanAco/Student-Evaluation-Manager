#ifndef DIALOG_H
#define DIALOG_H

#include <QtGui>
#include "CFGManager.h"

#define OPEN_FILE
#define NEW_FILE

class Dialog : public QDialog
{
    Q_OBJECT
public:
    Dialog(const QString& title, const QString& text,
           QWidget* parent = 0);
    ~Dialog();
protected:
    QVBoxLayout *layout;
    QHBoxLayout *buttons;
    QLabel *text;
    QLabel *result;
    QPushButton *ok;
    QPushButton *cancel;
signals:
    void okPressed();
private slots:
    void emitOKSignal();
};

class UserLoginDialog : public Dialog
{
    Q_OBJECT
public:
    UserLoginDialog(QWidget *parent = 0);
    void showMessage(const QString &msg);
    ~UserLoginDialog();
private:
    QLineEdit *usrnm;
    QLabel *password;
    QLineEdit *passwd;
signals:
    void dataChecked(const QString &name,
                     const QString &password);
private slots:
    void checkData();
};

class ErrorDialog : public Dialog
{
    Q_OBJECT
public:
    ErrorDialog(const QString& text, QWidget* parent = 0);
};

class FileDialog : public Dialog
{
    Q_OBJECT
public:
    FileDialog(const QString& title, const QString& text,
               QWidget* parent = 0);
    void loadTreeData(const QMap<QString, QString> &files,
                     const QMap<QString, QString> &folders);
    ~FileDialog();
public slots:
    virtual void checkValidity() = 0;
    void showSelectedItem();
signals:
    void dataChecked(const QString &name, int cols,
                     int rows, const QString &folder);
protected:
    QTreeWidget *treeView;
    QMap<QString, QString> *files;
    QMap<QString, QString> *folders;
    QLineEdit *fileName;
    bool treeDataContains(const QString &string);
    bool isFolder(const QString &nodeName);
    bool isFile(const QString &nodeName);
};

class OpenFileDialog : public FileDialog
{
    Q_OBJECT
public:
    OpenFileDialog(QWidget *parent = 0);
public slots:
    void checkValidity();
};

class NewFileDialog : public FileDialog
{
    Q_OBJECT
public:
    NewFileDialog(const CFGManager *cfg, QWidget *parent = 0);
    ~NewFileDialog();
public slots:
    void checkValidity();
private:
    QLineEdit *columnCount;
    QLineEdit *rowCount;
};

#endif // DIALOG_H

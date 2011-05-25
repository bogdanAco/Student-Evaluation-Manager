#ifndef TABLEDIALOG_H
#define TABLEDIALOG_H

#include "Dialog.h"

class TableDialog : public Dialog
{
    Q_OBJECT
public:
    TableDialog(const QString& title, const QString& text,
               QWidget* parent = 0);
    ~TableDialog();

public slots:
    void loadTreeData(const QList<QPair<QString, QString> > &tables,
                     const QList<QPair<QString, QString> > &folders);
    virtual void checkValidity() = 0;
    void showSelectedItem();

private slots:
    void deleteTable();
    void deleteFolder();
    void createFolder();
    void createFolder(const QString &name);

signals:
    void dataChecked(const QString &name, int cols,
                     int rows, const QString &folder);
    void okToDeleteTable(const QString &name);
    void okToDeleteFolder(const QString &name);
    void okToCreateFolder(const QString &name,
                          const QString &parent);
protected:
    QTreeWidget *treeView;
    void createTreeView();
    QMap<QString, QTreeWidgetItem*> *tables;
    QMap<QString, QTreeWidgetItem*> *folders;
    QLineEdit *tableName;
    QLabel *selectLabel;
    QAction *deleteTableAction;
    QAction *deleteFolderAction;
    QAction *createFolderAction;
    bool treeDataContains(const QString &string);
    bool isFolder(const QString &nodeName);
    bool isTable(const QString &nodeName);
};

class OpenTableDialog : public TableDialog
{
    Q_OBJECT
public:
    OpenTableDialog(QWidget *parent = 0);

public slots:
    void checkValidity();
};

class NewTableDialog : public TableDialog
{
    Q_OBJECT
public:
    NewTableDialog(const CFGManager *cfg, QWidget *parent = 0);
    ~NewTableDialog();

public slots:
    void checkValidity();

private:
    QLineEdit *columnCount;
    QLineEdit *rowCount;
};

class ImportDataDialog : public TableDialog
{
    Q_OBJECT
public:
    ImportDataDialog(QWidget *parent = 0);
    SpreadSheet *getSpreadsheet() const;
    ~ImportDataDialog();

public slots:
    void checkValidity();
    void getData();

private:
    SpreadSheet *table;
    QLineEdit *searchValue;
    QPushButton *search;

signals:
    void getDataSignal(int field, const QString &fieldVal,
                       const QString &table);
};

#endif // TABLEDIALOG_H

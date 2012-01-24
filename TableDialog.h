#ifndef TABLEDIALOG_H
#define TABLEDIALOG_H

#include "Dialog.h"

class TreeItem : public QTreeWidgetItem
{
public:
    enum TreeItemType { Table, Folder };
    
    TreeItem(TreeItemType type, QTreeWidget *parent = 0);
    TreeItem(TreeItemType type, QTreeWidgetItem *parent = 0);
    int getType();
    
private: 
    void initialize(TreeItemType type);
    TreeItemType type;
};

class TableDialog : public Dialog
{
    Q_OBJECT
public:
    TableDialog(const QString& title, const QString& text,
               QWidget* parent = 0);
    ~TableDialog();

public slots:
    void loadTreeData(const QHash<QString, QString> &tables,
                      const QHash<QString, QString> &folders);
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
    TreeItem *root;
    QList<QTreeWidgetItem*> removeTreeItemChildren(QTreeWidgetItem* item);
    void createTreeView();
    QLineEdit *tableName;
    QLabel *selectLabel;
    QAction *deleteTableAction;
    QAction *deleteFolderAction;
    QAction *createFolderAction;
    bool treeDataContains(const QString &string);
    bool isFolder(TreeItem *item);
    bool isTable(TreeItem *item);
    bool isFolder(const QString &nodeName);
    bool isTable(const QString &nodeName);
    void loadItemsByParent(QTreeWidgetItem *parent,
                             const QHash<QString,QString> &tables, 
                             const QHash<QString,QString> &folders);
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
    ImportDataDialog(const QMultiMap<int,int> &currentSelection, 
                     QWidget *parent = 0);
    SpreadSheet *getSpreadsheet() const;
    ~ImportDataDialog();

public slots:
    void checkValidity();
    void getData();
    void setRights();
    void setSelectedItems(const QMultiMap<int,int> &items);

private:
    SpreadSheet *table;
    QPushButton *load;
    QMultiMap<int,int> selection;
    QString table_name;

signals:
    void getDataSignal(const QString &table);
    void link(const QString &table, 
              const QMultiMap<int,int> &from, 
              const QMultiMap<int,int> &to);
};

#endif // TABLEDIALOG_H

#include "TableDialog.h"

TreeItem::TreeItem(TreeItemType type, QTreeWidget *parent) : QTreeWidgetItem(parent)
{
    initialize(type);
}

TreeItem::TreeItem(TreeItemType type, QTreeWidgetItem *parent) : QTreeWidgetItem(parent)
{
    initialize(type);
}

int TreeItem::getType()
{
    return this->type;
}

void TreeItem::initialize(TreeItemType type)
{
    this->type = type;
    if (type == Table)
        setIcon(0, QIcon("images/new.png"));
    else if (type == Folder)
        setIcon(0, QIcon("images/folder.png")); 
}

TableDialog::TableDialog(const QString &title, const QString &text,
                       QWidget* parent) : Dialog(title, text, parent)
{
    treeView = 0;
    createTreeView();

    tableName = new QLineEdit();
    tableName->setMaxLength(50);
    mainLayout->addWidget(tableName, 4, 0, 1, 3);

    selectLabel = new QLabel("Select the table:");
    mainLayout->addWidget(selectLabel, 7, 0, 1, 3);

    connect(this, SIGNAL(okPressed()), this, SLOT(checkValidity()));
}

void TableDialog::loadTreeData(const QHash<QString, QString> &tables, 
                               const QHash<QString, QString> &folders)
{
    QList<QTreeWidgetItem*> items = removeTreeItemChildren(root);
    for (int i=items.length()-1; i>=0; i--)
        delete items.at(i);
    
    loadItemsByParent(root, tables, folders);
    
    treeView->sortItems(0, Qt::AscendingOrder);
}

TableDialog::~TableDialog()
{
    delete selectLabel;
    delete deleteTableAction;
    delete deleteFolderAction;
    delete createFolderAction;
    delete treeView;
    delete tableName;
}

void TableDialog::showSelectedItem()
{
    TreeItem *selectedItem = (TreeItem*)treeView->currentItem();
    if (selectedItem != 0)
    {
        QString selectedItemName = selectedItem->text(0);
        if (isTable(selectedItem))
            tableName->setText(selectedItemName);
        else
            tableName->setText("");
    }
}

void TableDialog::deleteTable()
{
    showMessage("");
    TreeItem *selectedItem = (TreeItem*)treeView->currentItem();
    if (selectedItem == 0)
    {
        showMessage("No table selected");
        return;
    }
    QString selectedItemName = selectedItem->text(0);
    if (selectedItemName.length() == 0 || isFolder(selectedItemName))
    {
        showMessage("No table selected");
        return;
    }
    treeView->removeItemWidget(selectedItem, 0);
    delete selectedItem;
    emit okToDeleteTable(selectedItemName);
}

void TableDialog::deleteFolder()
{
    showMessage("");
    TreeItem *selectedItem = (TreeItem*)treeView->currentItem();
    if (selectedItem == 0)
    {
        showMessage("No folder selected");
        return;
    }
    QString selectedItemName = selectedItem->text(0);
    if (isTable(selectedItem) || selectedItemName.length() == 0)
    {
        showMessage("No folder selected");
        return;
    }
    if (selectedItemName == "Root")
    {
        showMessage("Cannot delete root folder");
        return;
    }
    emit okToDeleteFolder(selectedItemName);
}

void TableDialog::createFolder()
{
    CreateFolderDialog *diag = new CreateFolderDialog(this);
    diag->show();
    connect(diag, SIGNAL(selected(QString)),
            this, SLOT(createFolder(QString)));
}

void TableDialog::createFolder(const QString &name)
{
    showMessage("");
    if (name.length() == 0)
    {
        showMessage("Please enter a folder name");
        return;
    }
    if (isFolder(name))
    {
        showMessage("Folder already exist");
        return;
    }
    if (treeView->currentItem() == 0)
    {
        showMessage("Please select the folder's parent");
        return;
    }
    else
    {
        TreeItem *item = (TreeItem*)treeView->currentItem();
        if (isTable(item))
            emit okToCreateFolder(name, item->parent()->text(0));
        else
            emit okToCreateFolder(name, item->text(0));
    }
}

QList<QTreeWidgetItem*> TableDialog::removeTreeItemChildren(QTreeWidgetItem *item)
{
    QList<QTreeWidgetItem*> items;
    for (int i=0; i<item->childCount(); i++)
    {
        items.append(item->child(i));
        if (item->child(i)->childCount() > 0)
            items.append(removeTreeItemChildren(item->child(i)));
    }
    return items;
}

void TableDialog::createTreeView()
{
    treeView = new QTreeWidget(this);
    treeView->setColumnCount(1);
    treeView->setHeaderLabel("Tables");
    treeView->setContextMenuPolicy(Qt::ActionsContextMenu);
    mainLayout->addWidget(treeView, 8, 0, 1, 4);

    root = new TreeItem(TreeItem::Folder, treeView);
    root->setIcon(0, QIcon("images/folder.png"));
    root->setText(0, "Root");
    root->setExpanded(true);

    deleteTableAction = new QAction("Delete selected table", treeView);
    treeView->addAction(deleteTableAction);
    connect(deleteTableAction, SIGNAL(triggered()),
            this, SLOT(deleteTable()));

    deleteFolderAction = new QAction("Delete selected folder", treeView);
    treeView->addAction(deleteFolderAction);
    connect(deleteFolderAction, SIGNAL(triggered()),
            this, SLOT(deleteFolder()));

    createFolderAction = new QAction("Create new folder", treeView);
    treeView->addAction(createFolderAction);
    connect(createFolderAction, SIGNAL(triggered()),
            this, SLOT(createFolder()));

    connect(treeView, SIGNAL(itemSelectionChanged()),
            this, SLOT(showSelectedItem()));
}

bool TableDialog::treeDataContains(const QString &string)
{
    QList<QTreeWidgetItem*> findResult = treeView->findItems(string,
                                                             Qt::MatchExactly |
                                                             Qt::MatchRecursive);
    if (findResult.length() > 0)
        return true;
    return false;
}

bool TableDialog::isFolder(TreeItem *item)
{
    if (item->getType() == TreeItem::Folder)
        return true;
    else
        return false;
}

bool TableDialog::isTable(TreeItem *item)
{
    if (item->getType() == TreeItem::Table)
        return true;
    else
        return false;
}

bool TableDialog::isFolder(const QString &nodeName)
{
    QList<QTreeWidgetItem*> findResult = treeView->findItems(nodeName,
                                                      Qt::MatchExactly |
                                                      Qt::MatchRecursive);
    for (int n=0; n<findResult.length(); n++)
        if (((TreeItem*)findResult.at(n))->getType() == TreeItem::Folder)
            return true;
    return false;
}

bool TableDialog::isTable(const QString &nodeName)
{
    QList<QTreeWidgetItem*> findResult = treeView->findItems(nodeName,
                                                      Qt::MatchExactly |
                                                      Qt::MatchRecursive);
    for (int n=0; n<findResult.length(); n++)
        if (((TreeItem*)findResult.at(n))->getType() == TreeItem::Table)
            return true;
    return false;
}

void TableDialog::loadItemsByParent(QTreeWidgetItem *parent, 
                                    const QHash<QString, QString> &tables, 
                                    const QHash<QString, QString> &folders)
{
    QString txt = "";
    if (!parent)
        return;
    else if ((txt = parent->text(0)).isEmpty())
        return;
    
    QListIterator<QString> it(folders.keys(txt));
    while (it.hasNext())
    {
        QString folderName = it.next();
        TreeItem *item = new TreeItem(TreeItem::Folder, parent);
        item->setText(0, folderName);
        item->setExpanded(true);
        loadItemsByParent(item, tables, folders);
    }
    
    QListIterator<QString> filesIt(tables.keys(txt));
    while (filesIt.hasNext())
    {
        QString fileName = filesIt.next();
        TreeItem *item = new TreeItem(TreeItem::Table, parent);
        item->setText(0, fileName);
    }
}

OpenTableDialog::OpenTableDialog(QWidget *parent) :
        TableDialog("Open table", "Enter table name", parent)
{

}

void OpenTableDialog::checkValidity()
{
    showMessage("");
    int len = tableName->text().length();
    if (len == 0)
    {
        showMessage("No table name");
        return;
    }
    if (isFolder((TreeItem*)treeView->currentItem()))
    {
        showMessage("No table selected");
        return;
    }
    emit dataChecked(tableName->text(), 0, 0, "");
    this->close();
}

NewTableDialog::NewTableDialog(const CFGManager *cfg, QWidget *parent) :
        TableDialog("New table", "Enter new table name", parent)
{
    selectLabel->setText("Select the folder");

    mainLayout->addWidget(new QLabel("Columns"), 5, 0, 1, 2);
    columnCount = new QLineEdit(QString("%1").arg(cfg->getColumnCount()));
    columnCount->setMaxLength(2);
    mainLayout->addWidget(columnCount, 6, 0, 1, 2);
    mainLayout->addWidget(new QLabel("Rows:"), 5, 2, 1, 2);
    rowCount = new QLineEdit(QString("%1").arg(cfg->getRowCount()));
    rowCount->setMaxLength(4);
    mainLayout->addWidget(rowCount, 6, 2, 1, 2);
    
    disconnect(treeView, SIGNAL(itemSelectionChanged()),
               this, SLOT(showSelectedItem()));
}

NewTableDialog::~NewTableDialog()
{
    delete columnCount;
    delete rowCount;
}

void NewTableDialog::checkValidity()
{
    showMessage("");
    int len = tableName->text().length();
    if (len == 0)
    {
        showMessage("No table name");
        return;
    }
    if (treeView->currentItem() == 0)
    {
        showMessage("No folder selected");
        return;
    }
    if (isTable((TreeItem*)treeView->currentItem()))
    {
        showMessage("Invalid folder selected");
        return;
    }
    if (isTable(tableName->text()))
    {
        showMessage("Table name already exists");
        return;
    }
    if (tableName->text().contains(QRegExp("(;|:|\\)|\\()")))
    {
        showMessage("Table name must not contain ';', ':' or brackets");
        return;
    }
    bool ok;
    unsigned int cols = columnCount->text().toUInt(&ok);
    if (!ok || (cols == 0))
    {
        showMessage("Invalid column number");
        return;
    }
    unsigned int rows = rowCount->text().toUInt(&ok);
    if (!ok || (rows == 0))
    {
        showMessage("Invalid row number");
        return;
    }
    emit dataChecked(tableName->text(), cols, rows,
                     treeView->selectedItems()[0]->text(0));   
    this->close();
}

ImportDataDialog::ImportDataDialog(const QMultiMap<int, int> &currentSelection, QWidget *parent) :
    TableDialog("Import data", "Enter table name", parent)
{
    setMinimumSize(800, 600);
    table = new SpreadSheet(1, 1, this);
    table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mainLayout->addWidget(this->table, 0, 4, 10, 1);
    selection = currentSelection;
    table_name = "";
    
    load = new QPushButton("Load data");
    mainLayout->addWidget(load, 9, 0, 1, 3);
    connect(load, SIGNAL(pressed()), this, SLOT(getData()));
    
    ok->setEnabled(false);
}

SpreadSheet *ImportDataDialog::getSpreadsheet() const
{
    return table;
}

ImportDataDialog::~ImportDataDialog()
{
    delete table;
}

void ImportDataDialog::checkValidity()
{
    showMessage("");

    if (tableName->text().isEmpty())
    {
        showMessage("No document selected");
        return;
    }
    QMultiMap<int,int> selectedIndexes = table->selectedItemIndexes();
    if (!SpreadSheet::selectionsEquals(selection, selectedIndexes))
    {
        showMessage("Main selection size must be equal to "
                    "the size of the current selection");
        return;
    }
    emit link(table_name, selectedIndexes, selection);
    this->close();
}

void ImportDataDialog::getData()
{
    if (tableName->text().isEmpty())
    {
        showMessage("No document selected");
        return;
    }
    showMessage("");
    table_name = tableName->text();
    emit getDataSignal(tableName->text());
    ok->setEnabled(true);
}

void ImportDataDialog::setRights()
{
    table->setRights(QList<int>());
}

void ImportDataDialog::setSelectedItems(const QMultiMap<int, int> &items)
{
    selection = items;
}

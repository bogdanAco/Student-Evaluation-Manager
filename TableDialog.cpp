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
    
    QHashIterator<QString, QString> foldersIt(folders);
    while (foldersIt.hasNext())
    {
        foldersIt.next();
        QString folderName = foldersIt.key();
        QString parentName = foldersIt.value();
        
        TreeItem *item = 0;
        if (parentName == "")
            item = new TreeItem(TreeItem::Folder, root);
        else
        {
            QList<QTreeWidgetItem*> parentNodes = treeView->findItems(parentName,
                                                              Qt::MatchExactly |
                                                              Qt::MatchRecursive);
            if (parentNodes.length() <= 0)
                continue;
            item = new TreeItem(TreeItem::Folder, parentNodes.at(0));
        }
        if (!item)
            continue;
        item->setText(0, folderName);
        item->setExpanded(true);
    }
    
    QHashIterator<QString, QString> tablesIt(tables);
    while (tablesIt.hasNext())
    {
        tablesIt.next();
        QString tableName = tablesIt.key();
        QString parentName = tablesIt.value();
        
        TreeItem *item = 0;
        if (parentName == "Root")
            item = new TreeItem(TreeItem::Table, root);
        else
        {
            QList<QTreeWidgetItem*> parentNodes = treeView->findItems(parentName,
                                                              Qt::MatchExactly |
                                                              Qt::MatchRecursive);
            if (parentNodes.length() <= 0)
                continue;
            for (int n=0; n<parentNodes.length(); n++)
                if (((TreeItem*)parentNodes.at(n))->getType() == TreeItem::Folder)
                    item =  new TreeItem(TreeItem::Table, parentNodes.at(n));
            if (!item)
                continue;
        }
        item->setText(0, tableName);
    }
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
    if (isFolder(selectedItem) || selectedItemName.length() == 0)
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
    if (treeDataContains(name))
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
    if (isFolder(tableName->text()))
    {
        showMessage("No table selected");
        return;
    }
    emit dataChecked(tableName->text(), 0, 0, "");
    this->hide();
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
    if (treeView->selectedItems().length() == 0)
    {
        showMessage("No folder selected");
        return;
    }
    if (isTable((TreeItem*)treeView->selectedItems().at(0)))
    {
        showMessage("Invalid folder selected");
        return;
    }
    if (treeDataContains(tableName->text()))
    {
        showMessage("Table name already exists");
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
    this->hide();
}

ImportDataDialog::ImportDataDialog(QWidget *parent) :
    TableDialog("Import data", "Enter table name", parent)
{
    table = new SpreadSheet(6, 1, this);

    mainLayout->addWidget(new QLabel("Data from row:"), 0, 0, 1, 3);

    mainLayout->addWidget(new QLabel("Column:"), 0, 3, 1, 1);
    columnNo = new QLineEdit();
    mainLayout->addWidget(columnNo, 1, 3, 1, 1);

    searchValue = new QLineEdit();
    mainLayout->addWidget(searchValue, 1, 0, 1, 3);

    search = new QPushButton("Get data");
    mainLayout->addWidget(search, 6, 0, 1, 3);
    connect(search, SIGNAL(pressed()), this, SLOT(getData()));

    mainLayout->addWidget(this->table, 0, 4, 10, 1);
}

SpreadSheet *ImportDataDialog::getSpreadsheet() const
{
    return table;
}

ImportDataDialog::~ImportDataDialog()
{
    delete columnNo;
    delete searchValue;
    delete search;
}

void ImportDataDialog::checkValidity()
{
    showMessage("");

    if (table->currentItem()->text().length() == 0)
    {
        showMessage("No item selected");
        return;
    }

    //=link...
    this->hide();
}

void ImportDataDialog::getData()
{
    showMessage("");

    bool validCol;
    int column = columnNo->text().toInt(&validCol);
    if (!validCol)
    {
        showMessage("Please enter a number");
        return;
    }
    if (column > table->rowCount() || column <= 0)
    {
        showMessage("Number must between 1 "
                    "and table row number");
        return;
    }

    emit getDataSignal(column-1, searchValue->text(),
                       tableName->text());
}

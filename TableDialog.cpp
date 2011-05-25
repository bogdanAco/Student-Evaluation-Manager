#include "TableDialog.h"

TableDialog::TableDialog(const QString &title, const QString &text,
                       QWidget* parent) : Dialog(title, text, parent)
{
    tables = new QMap<QString, QTreeWidgetItem*>();
    folders = new QMap<QString, QTreeWidgetItem*>();

    treeView = 0;
    createTreeView();

    tableName = new QLineEdit();
    tableName->setMaxLength(50);
    mainLayout->addWidget(tableName, 4, 0, 1, 3);

    selectLabel = new QLabel("Select the table:");
    mainLayout->addWidget(selectLabel, 7, 0, 1, 3);

    connect(this, SIGNAL(okPressed()), this, SLOT(checkValidity()));
}

void TableDialog::loadTreeData(const QList<QPair<QString, QString> > &tables,
                               const QList<QPair<QString, QString> > &folders)
{
    this->tables->clear();
    this->folders->clear();
    treeView->hide();
    //delete treeView;
    createTreeView();

    for (int i=0; i<folders.length(); i++)
    {
        QString folderName = folders.at(i).first;
        QString parentName = folders.at(i).second;

        QTreeWidgetItem *item = 0;
        if (parentName == "")
            item = new QTreeWidgetItem(treeView);
        else if (this->folders->contains(parentName))
            item = new QTreeWidgetItem(this->folders->value(parentName));

        item->setIcon(0, QIcon("images/folder.png"));
        item->setText(0, folderName);
        item->setExpanded(true);
        this->folders->insert(folderName, item);
    }

    for (int i=0; i<tables.length(); i++)
    {
        QString tableName = tables.at(i).first;
        QString parentName = tables.at(i).second;

        if (this->folders->contains(parentName))
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(this->folders->value(parentName));
            item->setIcon(0, QIcon("images/new.png"));
            item->setText(0, tableName);
            this->tables->insert(tableName, item);
        }
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
    delete tables;
    delete folders;
    delete tableName;
}

void TableDialog::showSelectedItem()
{
    QString selectedItemName = treeView->selectedItems().at(0)->text(0);
    if (isTable(selectedItemName))
        tableName->setText(selectedItemName);
    else
        tableName->setText("");
}

void TableDialog::deleteTable()
{
    result->setText("");
    if (treeView->selectedItems().length() == 0)
    {
        result->setText("No table selected");
        return;
    }
    QString selectedItemName = treeView->selectedItems().at(0)->text(0);
    if (isFolder(selectedItemName) ||
        selectedItemName.length() == 0)
    {
        result->setText("No table selected");
        return;
    }
    emit okToDeleteTable(selectedItemName);
}

void TableDialog::deleteFolder()
{
    result->setText("");
    if (treeView->selectedItems().length() == 0)
    {
        result->setText("No folder selected");
        return;
    }
    QString selectedItemName = treeView->selectedItems().at(0)->text(0);
    if (isTable(selectedItemName) ||
        selectedItemName.length() == 0)
    {
        result->setText("No folder selected");
        return;
    }
    if (selectedItemName == "Root")
    {
        result->setText("Cannot delete root folder");
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
    result->setText("");
    if (name.length() == 0)
    {
        result->setText("Please enter a folder name");
        return;
    }
    if (treeDataContains(name))
    {
        result->setText("Folder already exist");
        return;
    }

    if (treeView->selectedItems().length() == 0)
        emit okToCreateFolder(name, "");
    else
    {
        QTreeWidgetItem *item = treeView->selectedItems().at(0);
        if (isTable(item->text(0)))
            emit okToCreateFolder(name, item->parent()->text(0));
        else
            emit okToCreateFolder(name, item->text(0));
    }
}

void TableDialog::createTreeView()
{
    treeView = new QTreeWidget(this);
    treeView->setColumnCount(1);
    treeView->setHeaderLabel("Tables");
    treeView->setContextMenuPolicy(Qt::ActionsContextMenu);
    mainLayout->addWidget(treeView, 8, 0, 1, 4);

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
    if (tables->contains(string))
        return true;
    if (folders->contains(string))
        return true;
    return false;
}

bool TableDialog::isFolder(const QString &nodeName)
{
    if (folders->contains(nodeName))
        return true;
    return false;
}

bool TableDialog::isTable(const QString &nodeName)
{
    if (tables->contains(nodeName))
        return true;
    return false;
}

OpenTableDialog::OpenTableDialog(QWidget *parent) :
        TableDialog("Open table", "Enter table name", parent)
{

}

void OpenTableDialog::checkValidity()
{
    result->setText("");
    int len = tableName->text().length();
    if (len == 0)
    {
        result->setText("No table name");
        return;
    }
    if (isFolder(tableName->text()))
    {
        result->setText("No table selected");
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
}

NewTableDialog::~NewTableDialog()
{
    delete columnCount;
    delete rowCount;
}

void NewTableDialog::checkValidity()
{
    result->setText("");
    int len = tableName->text().length();
    if (len == 0)
    {
        result->setText("No table name");
        return;
    }
    if (treeView->selectedItems().length() == 0)
    {
        result->setText("No folder selected");
        return;
    }
    if (isTable(treeView->selectedItems().at(0)->text(0)))
    {
        result->setText("Invalid folder selected");
        return;
    }
    if (treeDataContains(tableName->text()))
    {
        result->setText("Table name already exists");
        return;
    }
    bool ok;
    unsigned int cols = columnCount->text().toUInt(&ok);
    if (!ok || (cols == 0))
    {
        result->setText("Invalid column number");
        return;
    }
    unsigned int rows = rowCount->text().toUInt(&ok);
    if (!ok || (rows == 0))
    {
        result->setText("Invalid row number");
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

}

void ImportDataDialog::checkValidity()
{

}

void ImportDataDialog::getData()
{
    emit getDataSignal(0, searchValue->text(),
                       treeView->selectedItems().at(0)->text(0));
}

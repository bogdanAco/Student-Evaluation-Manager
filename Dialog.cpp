#include "Dialog.h"

Dialog::Dialog(const QString &title, const QString &text, QWidget* parent) :
        QDialog(parent)
{
    setMinimumWidth(200);
    layout = new QVBoxLayout(this);
    buttons = new QHBoxLayout();
    this->text = new QLabel(text,this);
    this->text->setWordWrap(true);
    setWindowTitle(title);
    layout->addWidget(this->text);
    layout->addStretch(1);

    result = new QLabel("",this);
    result->setStyleSheet("QLabel { color:red; }");
    layout->addWidget(result, 0, Qt::AlignRight);
    ok = new QPushButton(style()->standardIcon(QStyle::SP_DialogOkButton),"Ok",this);
    ok->setFixedWidth(75);
    buttons->addWidget(ok);
    cancel = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton),
                             "Cancel",this);
    cancel->setFixedWidth(75);
    connect(cancel,SIGNAL(clicked()),this,SLOT(close()));
    buttons->addWidget(cancel);
    layout->addLayout(buttons);
    layout->setAlignment(buttons, Qt::AlignRight);

    connect(ok, SIGNAL(clicked()), this, SLOT(emitOKSignal()));
}

Dialog::~Dialog()
{
    this->hide();
    delete text;
    delete buttons;
    delete layout;
}

void Dialog::emitOKSignal()
{
    emit okPressed();
}

UserLoginDialog::UserLoginDialog(QWidget *parent) :
        Dialog("User login","Username",parent)
{
    usrnm = new QLineEdit("admin", this);
    layout->insertWidget(layout->indexOf(text)+1, usrnm);
    password = new QLabel("Password:",this);
    layout->insertWidget(layout->indexOf(usrnm)+1, password);
    passwd = new QLineEdit("admin", this);
    passwd->setEchoMode(QLineEdit::Password);
    layout->insertWidget(layout->indexOf(password)+1, passwd);

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

FileDialog::FileDialog(const QString &title, const QString &text,
                       QWidget* parent) : Dialog(title, text, parent)
{
    treeView = new QTreeWidget(this);
    treeView->setColumnCount(1);
    treeView->setHeaderLabel("Files");
    layout->insertWidget(layout->indexOf(this->text)+1, treeView);

    fileName = new QLineEdit();
    fileName->setMaxLength(50);
    layout->insertWidget(layout->indexOf(this->text)+1, fileName);

    connect(this, SIGNAL(okPressed()), this, SLOT(checkValidity()));
}

void FileDialog::loadTreeData(const QMap<QString, QString> &files,
                 const QMap<QString, QString> &folders)
{
    this->files = new QMap<QString, QString>(files);
    this->folders = new QMap<QString, QString>(folders);

    QMap<QString, QString>::const_iterator i;
    for (i = this->folders->begin(); i != this->folders->end(); i++)
    {
        QTreeWidgetItem *item = 0;
        if (i.value() == "")
            item = new QTreeWidgetItem(treeView);
        else
        {
            QList<QTreeWidgetItem*> list = treeView->findItems(
                                            i.value(), Qt::MatchExactly);
            item = new QTreeWidgetItem(list[0]);
        }
        item->setIcon(0, QIcon("images/folder.png"));
        item->setText(0, i.key());
        item->setExpanded(true);
    }

    for (i = this->files->begin(); i != this->files->end(); i++)
    {
        QList<QTreeWidgetItem*> list = treeView->findItems(
                                        i.value(), Qt::MatchExactly);
        QTreeWidgetItem *item = new QTreeWidgetItem(list[0]);
        item->setIcon(0, QIcon("images/new.png"));
        item->setText(0, i.key());
    }
    treeView->sortItems(0, Qt::AscendingOrder);
}

void FileDialog::showSelectedItem()
{
    QString selectedItemName = treeView->selectedItems().at(0)->text(0);
    if (isFile(selectedItemName))
        fileName->setText(selectedItemName);
}

bool FileDialog::treeDataContains(const QString &string)
{
    if (files->contains(string))
        return true;
    if (folders->contains(string))
        return true;
    return false;
}

FileDialog::~FileDialog()
{
    delete treeView;
    delete files;
    delete folders;
    delete fileName;
}

bool FileDialog::isFolder(const QString &nodeName)
{
    if (folders->contains(nodeName))
        return true;
    return false;
}

bool FileDialog::isFile(const QString &nodeName)
{
    if (files->contains(nodeName))
        return true;
    return false;
}

OpenFileDialog::OpenFileDialog(QWidget *parent) :
        FileDialog("Open file", "Enter file name", parent)
{
    QLabel *label1 = new QLabel("Select the file:");
    layout->insertWidget(layout->indexOf(fileName)+1, label1);
    connect(treeView, SIGNAL(itemSelectionChanged()),
            this, SLOT(showSelectedItem()));
}

void OpenFileDialog::checkValidity()
{
    result->setText("");
    int len = fileName->text().length();
    if (len == 0)
    {
        result->setText("No file name");
        return;
    }
    if (isFolder(fileName->text()))
    {
        result->setText("No file selected");
        return;
    }
    emit dataChecked(fileName->text(), 0, 0, "");
    this->hide();
}

NewFileDialog::NewFileDialog(const CFGManager *cfg, QWidget *parent) :
        FileDialog("New file", "Enter new file name", parent)
{
    QLabel *label1 = new QLabel("Select the folder:");
    layout->insertWidget(layout->indexOf(fileName)+1, label1);

    QGridLayout *gl = new QGridLayout();
    QLabel *label2 = new QLabel("Columns:");
    gl->addWidget(label2, 0, 0);
    columnCount = new QLineEdit(QString("%1").arg(cfg->getColumnCount()));
    columnCount->setMaxLength(2);
    gl->addWidget(columnCount, 1, 0);
    QLabel *label3 = new QLabel("Rows:");
    gl->addWidget(label3, 0, 1);
    rowCount = new QLineEdit(QString("%1").arg(cfg->getRowCount()));
    rowCount->setMaxLength(4);
    gl->addWidget(rowCount, 1, 1);
    layout->insertLayout(layout->indexOf(fileName)+1, gl);
    connect(treeView, SIGNAL(itemSelectionChanged()),
            this, SLOT(showSelectedItem()));
}

NewFileDialog::~NewFileDialog()
{
    delete columnCount;
    delete rowCount;
}

void NewFileDialog::checkValidity()
{
    result->setText("");
    int len = fileName->text().length();
    if (len == 0)
    {
        result->setText("No file name");
        return;
    }
    if (treeView->selectedItems().length() == 0)
    {
        result->setText("No folder selected");
        return;
    }
    if (isFile(treeView->selectedItems().at(0)->text(0)))
    {
        result->setText("Invalid folder selected");
        return;
    }
    if (treeDataContains(fileName->text()))
    {
        result->setText("File name already exists");
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

    emit dataChecked(fileName->text(), cols, rows,
                     treeView->selectedItems()[0]->text(0));
    this->hide();
}

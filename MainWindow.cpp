#include "MainWindow.h"

MainWindow::MainWindow(const QString& title) : QMainWindow()
{
    resetSize();
    setWindowTitle(title);
    CreateActions();
    CreateMenus();
    CreateToolbars();
    show();

    dialog = 0;
    Spreadsheet = 0;
    connected = false;
    config = new CFGManager();
    DBcon = new DBManager(config);
    connect(DBcon, SIGNAL(queryError(QString)),
            this, SLOT(CreateErrorDialog(QString)));
    connect(DBcon, SIGNAL(initializeDatabaseRequest()),
            this, SLOT(initializeDatabase()));
    if(DBcon->connectDB(config->getDBUser(), config->getDBPassword()))
    {
        dialog = new UserLoginDialog(this);
        dialog->show();
        connect((UserLoginDialog*)dialog, SIGNAL(dataChecked(QString,QString)),
                DBcon, SLOT(login(QString,QString)));
        connect(DBcon, SIGNAL(loggedIn(int)), this, SLOT(logIn(int)));
    }
    else
        connected = false;
}

void MainWindow::resetSize()
{
    this->setMinimumSize(600, 600);
    this->resize(900, 600);
}

MainWindow::~MainWindow()
{
    delete Spreadsheet;
    delete dialog;
    delete DBcon;
    delete config;
    delete menuBar;
    delete tableToolBar;
    delete editToolBar;
    delete appToolBar;
    for (int i=0; i<tableActions.length(); i++)
        delete tableActions[i];
    for (int i=0; i<editActions.length(); i++)
        delete editActions[i];
    for (int i=0; i<appActions.length(); i++)
        delete appActions[i];
}

void MainWindow::CreateMenus()
{
    menuBar = new QMenuBar(this);
    table = new QMenu("&Table",menuBar);
    table->addActions(tableActions);
    table->insertSeparator(exportTableAction);
    edit = new QMenu("&Edit",menuBar);
    edit->addActions(editActions);
    application = new QMenu("&Application",menuBar);
    application->addActions(appActions);
    menuBar->addMenu(table);
    menuBar->addMenu(edit);
    menuBar->addMenu(application);
    menuBar->setMinimumWidth(700);
    this->setMenuBar(menuBar);
}

void MainWindow::CreateActions()
{
    newTableAction = new QAction(QIcon("images/new.png"),
                                "&New table",this);
    newTableAction->setShortcut(QKeySequence::New);
    newTableAction->setStatusTip("Create new table");
    connect(newTableAction,SIGNAL(triggered()),this,SLOT(newTable()));
    tableActions << newTableAction;

    openTableAction = new QAction(QIcon("images/open.png"),
                                 "&Open table",this);
    openTableAction->setShortcut(QKeySequence::Open);
    openTableAction->setStatusTip("Open a table");
    connect(openTableAction,SIGNAL(triggered()),this,SLOT(openTable()));
    tableActions << openTableAction;

    closeTableAction = new QAction(QIcon("images/remove.png"),
                                  "&Close table",this);
    closeTableAction->setShortcut(QKeySequence::Close);
    closeTableAction->setStatusTip("Close the current table");
    connect(closeTableAction,SIGNAL(triggered()),this,SLOT(closeTable()));
    tableActions << closeTableAction;

    importTableAction = new QAction(QIcon("images/new.png"),
                                    "&Import data",this);
    importTableAction->setStatusTip("Import data from another table");
    connect(importTableAction,SIGNAL(triggered()),this,SLOT(createImportDataDialog()));
    tableActions << importTableAction;

    exportTableAction = new QAction(QIcon("images/save.png"),
                                   "&Export PDF",this);
    exportTableAction->setShortcut(QKeySequence::Save);
    exportTableAction->setStatusTip("Export the current spreadsheet to PDF");
    connect(exportTableAction,SIGNAL(triggered()),this,SLOT(exportTable()));
    exportTableAction->setDisabled(true);
    tableActions << exportTableAction;

    copyAction = new QAction(QIcon("images/copy.png"),
                             "&Copy",this);
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setStatusTip("Copy the curent cell's content");
    connect(copyAction,SIGNAL(triggered()),this,SLOT(copy()));
    editActions << copyAction;

    cutAction = new QAction(QIcon("images/cut.png"),
                            "&Cut",this);
    cutAction->setShortcut(QKeySequence::Cut);
    cutAction->setStatusTip("Cut the curent cell's content");
    connect(cutAction,SIGNAL(triggered()),this,SLOT(cut()));
    editActions << cutAction;

    pasteAction = new QAction(QIcon("images/paste.png"),
                              "&Paste",this);
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setStatusTip("Cut the curent cell's content");
    connect(pasteAction,SIGNAL(triggered()),this,SLOT(paste()));
    editActions << pasteAction;

    deleteAction = new QAction(QIcon("images/delete.png"),
                               "&Delete",this);
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setStatusTip("Cut the curent cell's content");
    connect(deleteAction,SIGNAL(triggered()),this,SLOT(del()));
    editActions << deleteAction;

    addColumnAction = new QAction(QIcon("images/add.png"),
                               "&Add columns",this);
    addColumnAction->setStatusTip("Add new column");
    connect(addColumnAction,SIGNAL(triggered()),this,SLOT(createAddColumnsDialog()));
    editActions << addColumnAction;

    removeColumnAction = new QAction(QIcon("images/remove.png"),
                               "&Remove columns",this);
    removeColumnAction->setStatusTip("Remove selected columns");
    connect(removeColumnAction,SIGNAL(triggered()),this,SLOT(createRemoveColumnsDialog()));
    editActions << removeColumnAction;

    addRowAction = new QAction(QIcon("images/add.png"),
                               "&Add rows",this);
    addRowAction->setStatusTip("Add new row");
    connect(addRowAction,SIGNAL(triggered()),this,SLOT(createAddRowsDialog()));
    editActions << addRowAction;

    formulaAction = new QAction(QIcon("images/formula.png"), "Formula:", this);
    formulaAction->setStatusTip("Insert formula into cell");
    connect(formulaAction,SIGNAL(triggered()),this,SLOT(createFormulaDialog()));

    loginAction = new QAction(QIcon("images/user.png"),
                              "&Login",this);
    loginAction->setStatusTip("Login into application");
    connect(loginAction,SIGNAL(triggered()),this,SLOT(createLoginDialog()));
    appActions << loginAction;

    signinAction = new QAction(QIcon("images/user.png"),
                              "&Sign in",this);
    signinAction->setStatusTip("Create new user (must have the rights to do that)");
    connect(signinAction,SIGNAL(triggered()),this,SLOT(createSignInDialog()));
    appActions << signinAction;

    configureAction = new QAction(QIcon("images/settings.png"),
                                  "&Configuration",this);
    configureAction->setStatusTip("Configure application");
    connect(configureAction,SIGNAL(triggered()),this,SLOT(createConfigureAppDialog()));
    appActions << configureAction;
}

void MainWindow::CreateToolbars()
{
    tableToolBar = new QToolBar("&Table", this);
    tableToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    tableToolBar->setMovable(false);
    this->addToolBar(tableToolBar);
    tableToolBar->addActions(tableActions);
    tableToolBar->insertSeparator(importTableAction);
    tableToolBar->insertSeparator(exportTableAction);

    editToolBar = new QToolBar("&Edit", this);
    editToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    editToolBar->setMovable(false);
    this->addToolBar(editToolBar);
    editToolBar->addActions(editActions);
    editToolBar->insertSeparator(addColumnAction);

    appToolBar = new QToolBar("&Application", this);
    appToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    appToolBar->setMovable(false);
    this->addToolBar(appToolBar);
    appToolBar->addActions(appActions);

    this->addToolBarBreak();

    formulaToolBar = new QToolBar("Formula", this);
    formulaToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    formulaToolBar->setMovable(false);
    this->addToolBar(formulaToolBar);
    formulaToolBar->addAction(formulaAction);
    cellFormula = new QLineEdit();
    cellFormula->setFixedWidth(400);
    cellFormula->setDisabled(true);
    formulaToolBar->addWidget(cellFormula);
}

void MainWindow::CreateErrorDialog(const QString &message)
{
    /*
    if (dialog != 0)
        if (QString(dialog->metaObject()->className()) == "ErrorDialog")
            if (dialog->getText() == message)
                return; */

    disconnect(dialog);
    delete dialog;
    dialog = new ErrorDialog(message, this);
    dialog->show();
}

void MainWindow::newTable()
{
    if (!connected)
    {
        CreateErrorDialog("Please login first");
        return;
    }
    disconnect(dialog);
    delete dialog;
    dialog = new NewTableDialog(config, this);
    ((TableDialog*)dialog)->
            loadTreeData(DBcon->getTables(), DBcon->getFolders());
    connect((NewTableDialog*)dialog,
            SIGNAL(dataChecked(QString,int,int,QString)),
            DBcon, SLOT(createTable(QString,int,int,QString)));
    connect(DBcon, SIGNAL(tableCreated(QString,int,int)),
            this, SLOT(createNewTable(QString,int,int)));

    connect((TableDialog*)dialog, SIGNAL(okToDeleteTable(QString)),
            DBcon, SLOT(removeTable(QString)));
    connect((TableDialog*)dialog, SIGNAL(okToDeleteFolder(QString)),
            DBcon, SLOT(removeFolder(QString)));
    connect((TableDialog*)dialog, SIGNAL(okToCreateFolder(QString, QString)),
            DBcon, SLOT(createFolder(QString, QString)));
    connect(DBcon, SIGNAL(dataModified(QList<QPair<QString,QString> >,
                                        QList<QPair<QString,QString> >)),
            (TableDialog*)dialog, SLOT(loadTreeData(QList<QPair<QString,QString> >,
                                                    QList<QPair<QString,QString> >)));
    dialog->show();
}

void MainWindow::createNewTable(const QString &name, int columns, int rows)
{
    Spreadsheet = new SpreadSheet(rows, columns, this);
    Spreadsheet->setCellsSize(config->getCellsSize());
    setCentralWidget(Spreadsheet);
    Spreadsheet->addActions(editActions);
    DBcon->setCurrentSpreadSheet(Spreadsheet);

    connect(Spreadsheet, SIGNAL(modified(QString)),
            this, SLOT(writeToDB(QString)));
    connect(Spreadsheet, SIGNAL(invalidFormula(QString)),
            this, SLOT(CreateErrorDialog(QString)));

    setWindowTitle(QString("%1 - %2").arg(name, windowTitle()));

    exportTableAction->setEnabled(true);
    int size = columns * 110;
    if (size < QApplication::desktop()->width())
        this->setMinimumWidth(size);
}

void MainWindow::openTable()
{
    if (!connected)
    {
        CreateErrorDialog("Please login first");
        return;
    }
    disconnect(dialog);
    delete dialog;
    dialog = new OpenTableDialog(this);
    ((TableDialog*)dialog)->
            loadTreeData(DBcon->getTables(), DBcon->getFolders());
    connect((OpenTableDialog*)dialog,
            SIGNAL(dataChecked(QString,int,int,QString)),
            DBcon, SLOT(openTable(QString,int,int,QString)));
    connect(DBcon, SIGNAL(tableOpened(QString,int,int)),
            this, SLOT(openNewTable(QString,int,int)));

    connect((TableDialog*)dialog, SIGNAL(okToDeleteTable(QString)),
            DBcon, SLOT(removeTable(QString)));
    connect((TableDialog*)dialog, SIGNAL(okToDeleteFolder(QString)),
            DBcon, SLOT(removeFolder(QString)));
    connect((TableDialog*)dialog, SIGNAL(okToCreateFolder(QString, QString)),
            DBcon, SLOT(createFolder(QString, QString)));
    connect(DBcon, SIGNAL(dataModified(QList<QPair<QString,QString> >,
                                        QList<QPair<QString,QString> >)),
            (TableDialog*)dialog, SLOT(loadTreeData(QList<QPair<QString,QString> >,
                                                    QList<QPair<QString,QString> >)));
    dialog->show();
}

void MainWindow::openNewTable(const QString &name, int columns, int rows)
{
    Spreadsheet = new SpreadSheet(rows, columns, this);
    Spreadsheet->setCellsSize(config->getCellsSize());
    setCentralWidget(Spreadsheet);
    Spreadsheet->addActions(editActions);
    DBcon->setCurrentSpreadSheet(Spreadsheet);

    connect(Spreadsheet, SIGNAL(modified(QString)),
            this, SLOT(writeToDB(QString)));
    connect(Spreadsheet, SIGNAL(invalidFormula(QString)),
            this, SLOT(CreateErrorDialog(QString)));

    setWindowTitle(QString("%1 - %2").arg(name, windowTitle()));

    cellFormula->setEnabled(true);
    connect(cellFormula, SIGNAL(returnPressed()),
            this, SLOT(setFormula()));
    connect(Spreadsheet, SIGNAL(cellClicked(int,int)),
            this, SLOT(showFormula(int,int)));

    exportTableAction->setEnabled(true);
    int size = columns * 110;
    if (size < QApplication::desktop()->width())
        this->setMinimumWidth(size);
}

void MainWindow::closeTable()
{
    if (!connected || Spreadsheet == 0)
    {
        if (!connected)
            CreateErrorDialog("Please login first");
        else if (Spreadsheet == 0)
            CreateErrorDialog("No table opened");
        return;
    }
    disconnect(dialog);
    delete dialog;
    dialog = new Dialog("Close table",
             "Are you sure you want to close the table ?",
             this);
    connect(dialog, SIGNAL(okPressed()), this, SLOT(closeOpenedTable()));
    dialog->show();
}

void MainWindow::closeOpenedTable()
{
    setWindowTitle("Student Evaluation Manager");
    DBcon->removeCurrentData();
    delete Spreadsheet;
    Spreadsheet = 0;
    dialog->hide();
    exportTableAction->setDisabled(true);
    cellFormula->setText("");
    cellFormula->setDisabled(true);
    resetSize();
}

void MainWindow::exportTable()
{
    QString table = QFileDialog::getSaveFileName(this);
    if (!Spreadsheet->printSpreadSheet(table))
    {
        disconnect(dialog);
        dialog = new ErrorDialog("PDF export error - "
                                 "append .pdf to table name", this);
        dialog->show();
    }
}

void MainWindow::cut()
{
    if (!connected || Spreadsheet == 0)
    {
        if (!connected)
            CreateErrorDialog("Please login first");
        else if (Spreadsheet == 0)
            CreateErrorDialog("No table opened");
        return;
    }
    Spreadsheet->cut();
    cellFormula->setText("");
}

void MainWindow::copy()
{
    if (!connected || Spreadsheet == 0)
    {
        if (!connected)
            CreateErrorDialog("Please login first");
        else if (Spreadsheet == 0)
            CreateErrorDialog("No table opened");
        return;
    }
    Spreadsheet->copy();
}

void MainWindow::paste()
{
    if (!connected || Spreadsheet == 0)
    {
        if (!connected)
            CreateErrorDialog("Please login first");
        else if (Spreadsheet == 0)
            CreateErrorDialog("No table opened");
        return;
    }
    Spreadsheet->paste();
}

void MainWindow::del()
{
    if (!connected || Spreadsheet == 0)
    {
        if (!connected)
            CreateErrorDialog("Please login first");
        else if (Spreadsheet == 0)
            CreateErrorDialog("No table opened");
        return;
    }
    Spreadsheet->del();
    cellFormula->setText("");
}

void MainWindow::writeToDB(const QString &cellData)
{
    QStringList aux = cellData.split('\n');
    QVariant line = aux.at(0);
    QVariant col = aux.at(1);
    DBcon->writeData(line.toInt(), col.toInt(), aux.at(2));
}

void MainWindow::showFormula(int row, int column)
{
    cellFormula->setText(Spreadsheet->currentFormula());
}

void MainWindow::setFormula()
{
    if (cellFormula->text().length() > 0)
        Spreadsheet->setFormula(cellFormula->text());
}

void MainWindow::logIn(int uid)
{
    if (uid == -1)
    {
        ((UserLoginDialog*)dialog)->showMessage("Invalid login data");
        return;
    }
    connected = true;
    dialog->hide();
}

void MainWindow::createAddColumnsDialog()
{
    if (!connected || Spreadsheet == 0)
    {
        if (!connected)
            CreateErrorDialog("Please login first");
        else if (Spreadsheet == 0)
            CreateErrorDialog("No table opened");
        return;
    }
    disconnect(dialog);
    delete dialog;
    dialog = new ModifyDialog("Add columns", this);
    dialog->show();
    connect((ModifyDialog*)dialog, SIGNAL(dataChecked(int)),
            DBcon, SLOT(addColumns(int)));
    connect(DBcon, SIGNAL(columnsAdded(int)),
            this, SLOT(resizeWindow(int)));
}

void MainWindow::createRemoveColumnsDialog()
{
    if (!connected || Spreadsheet == 0)
    {
        if (!connected)
            CreateErrorDialog("Please login first");
        else if (Spreadsheet == 0)
            CreateErrorDialog("No table opened");
        return;
    }
    int ok = QMessageBox::question(this, "Remove columns",
                                   "Are you sure you want to "
                                   "remove the selected columns ?",
                                   QMessageBox::Yes, QMessageBox::No);
    if (ok == QMessageBox::Yes)
        DBcon->removeColumns(Spreadsheet->selectedColumns());
}

void MainWindow::createAddRowsDialog()
{
    if (!connected || Spreadsheet == 0)
    {
        if (!connected)
            CreateErrorDialog("Please login first");
        else if (Spreadsheet == 0)
            CreateErrorDialog("No table opened");
        return;
    }
    disconnect(dialog);
    delete dialog;
    dialog = new ModifyDialog("Add rows", this);
    dialog->show();
    connect((ModifyDialog*)dialog, SIGNAL(dataChecked(int)),
            DBcon, SLOT(addRows(int)));
    connect(DBcon, SIGNAL(rowsAdded(int)),
            this, SLOT(resizeWindow(int)));
}

void MainWindow::createConfigureAppDialog()
{
    disconnect(dialog);
    delete dialog;
    dialog = new ConfigurationDialog(config, this);
    connect((ConfigurationDialog*)dialog, SIGNAL(changeKey(QString,QString)),
            DBcon, SLOT(changeKey(QString,QString)));
    connect((ConfigurationDialog*)dialog, SIGNAL(changePKey(QString,QString)),
            DBcon, SLOT(changePKey(QString,QString)));
    connect(DBcon, SIGNAL(message(QString)),
            dialog, SLOT(showMessage(QString)));
    dialog->show();
}

void MainWindow::createImportDataDialog()
{
    if (!connected || Spreadsheet == 0)
    {
        if (!connected)
            CreateErrorDialog("Please login first");
        else if (Spreadsheet == 0)
            CreateErrorDialog("No table opened");
        return;
    }
    disconnect(dialog);
    delete dialog;
    dialog = new ImportDataDialog(this);
    connect((ImportDataDialog*)dialog,
            SIGNAL(getDataSignal(int,QString,QString)),
            DBcon, SLOT(getGivenData(int,QString,QString)));
    connect(DBcon, SIGNAL(givenDataLoaded(QStringList)),
            ((ImportDataDialog*)dialog)->getSpreadsheet(),
            SLOT(loadData(QStringList)));

    ((TableDialog*)dialog)->
            loadTreeData(DBcon->getTables(), DBcon->getFolders());
    dialog->show();
}

void MainWindow::createFormulaDialog()
{
    if (!connected || Spreadsheet == 0)
    {
        if (!connected)
            CreateErrorDialog("Please login first");
        else if (Spreadsheet == 0)
            CreateErrorDialog("No table opened");
        return;
    }
    disconnect(dialog);
    delete dialog;
    dialog = new FormulaDialog(Spreadsheet->currentRow(),
                               Spreadsheet->currentColumn(),
                               Spreadsheet, this);
    connect((FormulaDialog*)dialog, SIGNAL(setFormula(int,int,QString)),
            Spreadsheet, SLOT(setFormula(int,int,QString)));
    dialog->show();
}

void MainWindow::createLoginDialog()
{
    if (connected)
    {
        CreateErrorDialog("Already connected");
        return;
    }
    disconnect(dialog);
    delete dialog;
    dialog = new UserLoginDialog(this);
    dialog->show();
    connect((UserLoginDialog*)dialog, SIGNAL(dataChecked(QString,QString)),
            DBcon, SLOT(login(QString,QString)));
    connect(DBcon, SIGNAL(loggedIn(int)), this, SLOT(logIn(int)));
}

void MainWindow::createSignInDialog()
{
    if (!connected)
    {
        CreateErrorDialog("Please login first");
        return;
    }
    disconnect(dialog);
    delete dialog;
    dialog = new UserSignInDialog(this);
    dialog->show();
    connect((UserSignInDialog*)dialog, SIGNAL(dataChecked(QString,QString,QString)),
            DBcon, SLOT(createUser(QString,QString,QString)));
    connect((UserSignInDialog*)dialog, SIGNAL(generateKeyFile(QString,QString)),
            config, SLOT(saveUserKey(QString,QString)));
    connect(DBcon, SIGNAL(userCreated()), dialog, SLOT(hide()));
}

void MainWindow::resizeWindow(int cells)
{
    int size = Spreadsheet->columnCount() * 110;
    if (size < QApplication::desktop()->width())
        this->setMinimumWidth(size);
}

void MainWindow::initializeDatabase()
{
    int ok = QMessageBox::question(this, "Initialization",
                                   "Database not found\n"
                                   "Do you want to create a new database\n"
                                   "using the name from configuration ?",
                                   QMessageBox::Yes, QMessageBox::No);
    if (ok == QMessageBox::Yes)
        DBcon->initializeDatabase();
}

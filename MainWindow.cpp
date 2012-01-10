#include "MainWindow.h"

MainWindow::MainWindow(const QString& title) : QMainWindow()
{
    resetSize();
    setWindowTitle(title);
    showMaximized();
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
    connect(DBcon, SIGNAL(closeCurrentTable()),
            this, SLOT(closeOpenedTable()));
    if(DBcon->connectDB(config->getDBUser(), config->getDBPassword()))
        createLoginDialog();
    else
        connected = false;
}

void MainWindow::resetSize()
{
    setMinimumSize(600, 500);
    if (!isMaximized())
        resize(700, 500);
}

MainWindow::~MainWindow()
{
    delete Spreadsheet;
    delete dialog;
    delete DBcon;
    delete config;
    delete menuBar;
    delete timer;
    delete statusMsg;
    delete status;
    delete tableToolBar;
    delete editToolBar;
    delete appToolBar;
    for (int i=0; i<tableActions.length(); i++)
        delete tableActions[i];
    for (int i=0; i<editActions.length(); i++)
        delete editActions[i];
    for (int i=0; i<appActions.length(); i++)
        delete appActions[i];
    delete colorPixmap;
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
    connect(newTableAction,SIGNAL(triggered()),this,SLOT(newTable()));
    tableActions << newTableAction;

    openTableAction = new QAction(QIcon("images/open.png"),
                                 "&Open table",this);
    openTableAction->setShortcut(QKeySequence::Open);
    connect(openTableAction,SIGNAL(triggered()),this,SLOT(openTable()));
    tableActions << openTableAction;

    closeTableAction = new QAction(QIcon("images/remove.png"),
                                  "&Close table",this);
    closeTableAction->setShortcut(QKeySequence::Close);
    connect(closeTableAction,SIGNAL(triggered()),this,SLOT(closeTable()));
    tableActions << closeTableAction;

    importTableAction = new QAction(QIcon("images/new.png"),
                                    "&Import data",this);
    connect(importTableAction,SIGNAL(triggered()),this,SLOT(createImportDataDialog()));
    tableActions << importTableAction;

    exportTableAction = new QAction(QIcon("images/save.png"),
                                   "&Export PDF",this);
    exportTableAction->setShortcut(QKeySequence::Save);
    connect(exportTableAction,SIGNAL(triggered()),this,SLOT(exportTable()));
    exportTableAction->setDisabled(true);
    tableActions << exportTableAction;

    copyAction = new QAction(QIcon("images/copy.png"),
                             "&Copy",this);
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction,SIGNAL(triggered()),this,SLOT(copy()));
    editActions << copyAction;

    cutAction = new QAction(QIcon("images/cut.png"),
                            "&Cut",this);
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction,SIGNAL(triggered()),this,SLOT(cut()));
    editActions << cutAction;

    pasteAction = new QAction(QIcon("images/paste.png"),
                              "&Paste",this);
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction,SIGNAL(triggered()),this,SLOT(paste()));
    editActions << pasteAction;

    deleteAction = new QAction(QIcon("images/delete.png"),
                               "&Delete",this);
    deleteAction->setShortcut(QKeySequence::Delete);
    connect(deleteAction,SIGNAL(triggered()),this,SLOT(del()));
    editActions << deleteAction;

    addColumnAction = new QAction(QIcon("images/add.png"),
                               "&Add columns",this);
    connect(addColumnAction,SIGNAL(triggered()),this,SLOT(createAddColumnsDialog()));
    editActions << addColumnAction;

    removeColumnAction = new QAction(QIcon("images/remove.png"),
                               "&Remove columns",this);
    connect(removeColumnAction,SIGNAL(triggered()),this,SLOT(createRemoveColumnsDialog()));
    editActions << removeColumnAction;

    addRowAction = new QAction(QIcon("images/add.png"),
                               "&Add rows",this);
    connect(addRowAction,SIGNAL(triggered()),this,SLOT(createAddRowsDialog()));
    editActions << addRowAction;

    grantReadAccessAction = new QAction(QIcon("images/user.png"),
                                        "&Grant read access", this);
    connect(grantReadAccessAction, SIGNAL(triggered()),
            this, SLOT(createGrantReadAccessDialog()));
    editActions << grantReadAccessAction;
    
    grantWriteAccessAction = new QAction(QIcon("images/user.png"),
                               "&Grant write access", this);
    connect(grantWriteAccessAction, SIGNAL(triggered()),
            this, SLOT(createGrantWriteAccessDialog()));
    editActions << grantWriteAccessAction;

    setHeaderTextAction = new QAction(QIcon("images/settings.png"),
                               "&Set column header text",this);
    connect(setHeaderTextAction,SIGNAL(triggered()),this,SLOT(createSetColumnHeaderDialog()));
    editActions << setHeaderTextAction;

    resetHeaderTextAction = new QAction(QIcon("images/settings.png"),
                               "&Reset column header text",this);
    connect(resetHeaderTextAction,SIGNAL(triggered()),this,SLOT(createResetColumnHeaderDialog()));
    editActions << resetHeaderTextAction;

    formulaAction = new QAction(QIcon("images/formula.png"), "Formula:", this);
    connect(formulaAction,SIGNAL(triggered()),this,SLOT(createFormulaDialog()));

    loginAction = new QAction(QIcon("images/user.png"),
                              "&Login",this);
    connect(loginAction,SIGNAL(triggered()),this,SLOT(createLoginDialog()));
    appActions << loginAction;

    logoutAction = new QAction(QIcon("images/user.png"),
                              "&Logout",this);
    connect(logoutAction,SIGNAL(triggered()),this,SLOT(logOut()));
    appActions << logoutAction;

    signinAction = new QAction(QIcon("images/user.png"),
                              "&Sign in",this);
    connect(signinAction,SIGNAL(triggered()),this,SLOT(createSignInDialog()));
    appActions << signinAction;

    configureAction = new QAction(QIcon("images/settings.png"),
                                  "&Configuration",this);
    connect(configureAction,SIGNAL(triggered()),this,SLOT(createConfigureAppDialog()));
    appActions << configureAction;
}

void MainWindow::CreateToolbars()
{
    tableToolBar = new QToolBar("&Table", this);
    //tableToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    tableToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    tableToolBar->setMovable(false);
    this->addToolBar(tableToolBar);
    tableToolBar->addActions(tableActions);
    tableToolBar->insertSeparator(importTableAction);
    tableToolBar->insertSeparator(exportTableAction);

    editToolBar = new QToolBar("&Edit", this);
    //editToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    editToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    editToolBar->setMovable(false);
    this->addToolBar(editToolBar);
    editToolBar->addActions(editActions);
    
    fontButton = new QPushButton();
    connect(fontButton, SIGNAL(clicked()),
            this, SLOT(createSetFontDialog()));
    colorPixmap = new QPixmap(10, 10);
    fontColorButton = new QPushButton("Font color");
    connect(fontColorButton, SIGNAL(clicked()),
            this, SLOT(createSetFontColorDialog()));
    colorPixmap->fill(Qt::black);
    fontColorButton->setIcon(QIcon(*colorPixmap));
    backgroundColorButton = new QPushButton("Background color");
    connect(backgroundColorButton, SIGNAL(clicked()),
            this, SLOT(createSetBackgroundColorDialog()));
    colorPixmap->fill(Qt::white);
    backgroundColorButton->setIcon(QIcon(*colorPixmap));
    editToolBar->addWidget(fontButton);
    editToolBar->addWidget(fontColorButton);
    editToolBar->addWidget(backgroundColorButton);
    editToolBar->insertSeparator(addColumnAction);

    appToolBar = new QToolBar("&Application", this);
    //appToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    appToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
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
    
    status = new QStatusBar(this);
    statusMsg = new QLabel(status);
    statusMsg->setStyleSheet("QLabel { color:red; }");
    status->addPermanentWidget(statusMsg, 1);
    this->setStatusBar(status);
    timer = new QTimer(this);
    timer->start(5000);
    connect(timer, SIGNAL(timeout()), this, SLOT(clearErrorMessage()));
}

void MainWindow::CreateErrorDialog(const QString &message)
{
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
    connect(DBcon, SIGNAL(dataModified(QHash<QString,QString>,
                                       QHash<QString,QString>)),
            ((TableDialog*)dialog), SLOT(loadTreeData(QHash<QString,QString>,
                                                      QHash<QString,QString>)));
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
            this, SLOT(displayError(QString)));
    connect(Spreadsheet, SIGNAL(currentSelectionChanged(QFont,QBrush,QBrush)),
            this, SLOT(displayCurrentCellSettings(QFont,QBrush,QBrush)));

    setWindowTitle(QString("%1 - %2").arg(name, "Student Evaluation Manager"));
    
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

void MainWindow::openTable()
{
    if (!connected)
    {
        CreateErrorDialog("Please login first");
        return;
    }
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
    connect(DBcon, SIGNAL(dataModified(QHash<QString,QString>,
                                       QHash<QString,QString>)),
            ((TableDialog*)dialog), SLOT(loadTreeData(QHash<QString,QString>,
                                                      QHash<QString,QString>)));
    dialog->show();
}

void MainWindow::openNewTable(const QString &name, int columns, int rows)
{
    Spreadsheet = new SpreadSheet(rows, columns, this);
    Spreadsheet->setCellsSize(config->getCellsSize());
    Spreadsheet->setRefreshTime(config->getRefreshTime());
    Spreadsheet->setSelectionMode(config->getSelectionMode());
    setCentralWidget(Spreadsheet);
    Spreadsheet->addActions(editActions);
    DBcon->setCurrentSpreadSheet(Spreadsheet);

    connect(Spreadsheet, SIGNAL(modified(QString)),
            this, SLOT(writeToDB(QString)));
    connect(Spreadsheet, SIGNAL(invalidFormula(QString)),
            this, SLOT(displayError(QString)));
    connect(Spreadsheet, SIGNAL(currentSelectionChanged(QFont,QBrush,QBrush)),
            this, SLOT(displayCurrentCellSettings(QFont,QBrush,QBrush)));

    setWindowTitle(QString("%1 - %2").arg(name, "Student Evaluation Manager"));

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
    if (table == "")
        return;

    if (!Spreadsheet->printSpreadSheet(table))
    {
        delete dialog;
        dialog = new ErrorDialog("PDF export error", this);
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
    int line = aux.at(0).toInt();
    int col = aux.at(1).toInt();
    QString data = aux.at(2);
    DBcon->writeData(line, col, data);
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

void MainWindow::logOut()
{
    if (connected)
    {
        connected = false;
        closeOpenedTable();
    }
    else
        CreateErrorDialog("Not logged in");
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
    delete dialog;
    dialog = new ModifyDialog("Add rows", this);
    dialog->show();
    connect((ModifyDialog*)dialog, SIGNAL(dataChecked(int)),
            DBcon, SLOT(addRows(int)));
    connect(DBcon, SIGNAL(rowsAdded(int)),
            this, SLOT(resizeWindow(int)));
}

void MainWindow::createSetColumnHeaderDialog()
{
    if (!connected || Spreadsheet == 0)
    {
        if (!connected)
            CreateErrorDialog("Please login first");
        else if (Spreadsheet == 0)
            CreateErrorDialog("No table opened");
        return;
    }
    delete dialog;
    dialog = new TextModifyDialog("Set current column's header text:", this);
    dialog->show();
    connect((TextModifyDialog*)dialog, SIGNAL(dataChecked(QString)),
            Spreadsheet, SLOT(setCurrentColumnHeaderText(QString)));
    connect(Spreadsheet, SIGNAL(columnHeaderTextChanged(int,QString)),
            DBcon, SLOT(setColumnHeaderText(int,QString)));
}

void MainWindow::createResetColumnHeaderDialog()
{
    if (!connected || Spreadsheet == 0)
    {
        if (!connected)
            CreateErrorDialog("Please login first");
        else if (Spreadsheet == 0)
            CreateErrorDialog("No table opened");
        return;
    }
    connect(Spreadsheet, SIGNAL(columnHeaderTextChanged(int,QString)),
            DBcon, SLOT(setColumnHeaderText(int,QString)));
    int ok = QMessageBox::question(this, "Reset column header text",
                                   "Are you sure you want to "
                                   "reset the current column's header text ?",
                                   QMessageBox::Yes, QMessageBox::No);
    if (ok == QMessageBox::Yes)
        Spreadsheet->setCurrentColumnHeaderText("");
}

void MainWindow::createSetFontDialog()
{
    if (!connected || Spreadsheet == 0)
    {
        if (!connected)
            CreateErrorDialog("Please login first");
        else if (Spreadsheet == 0)
            CreateErrorDialog("No table opened");
        return;
    }

    bool ok;    
    QFont f = QFontDialog::getFont(&ok, Spreadsheet->currentFont(), this);
    if (ok)
    {
        Spreadsheet->setCurrentCellsFont(f);
        fontButton->setText(f.family());
    }
}

void MainWindow::createSetFontColorDialog()
{
    if (!connected || Spreadsheet == 0)
    {
        if (!connected)
            CreateErrorDialog("Please login first");
        else if (Spreadsheet == 0)
            CreateErrorDialog("No table opened");
        return;
    }
    
    QColor c = QColorDialog::getColor(Qt::black, this);
    Spreadsheet->setFontColor(c);
    colorPixmap->fill(c);
    fontColorButton->setIcon(QIcon(*colorPixmap));
}

void MainWindow::createSetBackgroundColorDialog()
{
    if (!connected || Spreadsheet == 0)
    {
        if (!connected)
            CreateErrorDialog("Please login first");
        else if (Spreadsheet == 0)
            CreateErrorDialog("No table opened");
        return;
    }
    
    QColor c = QColorDialog::getColor(Qt::white, this);
    Spreadsheet->setBackgroundColor(c);
    colorPixmap->fill(c);
    backgroundColorButton->setIcon(QIcon(*colorPixmap));
}

void MainWindow::setCurrentCellsFont(const QFont &font)
{
    if (!connected || Spreadsheet == 0)
    {
        if (!connected)
            CreateErrorDialog("Please login first");
        else if (Spreadsheet == 0)
            CreateErrorDialog("No table opened");
        return;
    }
    
    Spreadsheet->setCurrentCellsFont(font);
}

void MainWindow::createGrantReadAccessDialog()
{
    if (!connected || Spreadsheet == 0)
    {
        if (!connected)
            CreateErrorDialog("Please login first");
        else if (Spreadsheet == 0)
            CreateErrorDialog("No table opened");
        return;
    }
    delete dialog;
    dialog = new TextModifyDialog("Grant rights for selected columns to user:", this);
    dialog->show();
    connect((TextModifyDialog*)dialog, SIGNAL(dataChecked(QString)),
            DBcon, SLOT(grantReadAccess(QString)));
}

void MainWindow::createGrantWriteAccessDialog()
{
    if (!connected || Spreadsheet == 0)
    {
        if (!connected)
            CreateErrorDialog("Please login first");
        else if (Spreadsheet == 0)
            CreateErrorDialog("No table opened");
        return;
    }
    delete dialog;
    dialog = new TextModifyDialog("Grant rights for selected columns to user:", this);
    dialog->show();
    connect((TextModifyDialog*)dialog, SIGNAL(dataChecked(QString)),
            this, SLOT(grantWriteAccess(QString)));
}

void MainWindow::grantWriteAccess(const QString &username)
{
    DBcon->grantWriteAccess(username, Spreadsheet->selectedColumns());
}

void MainWindow::createConfigureAppDialog()
{
    delete dialog;
    dialog = new ConfigurationDialog(config, this);
    connect(DBcon, SIGNAL(message(QString)),
            dialog, SLOT(showMessage(QString)));
    connect((ConfigurationDialog*)dialog, SIGNAL(changeKeys(QString,QString,QString,QString)),
            DBcon, SLOT(changeKey(QString,QString,QString,QString)));
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
    delete dialog;
    dialog = new UserLoginDialog(this);
    dialog->show();
    connect((UserLoginDialog*)dialog, SIGNAL(dataChecked(QString,QString)),
            DBcon, SLOT(login(QString,QString)));
    /*
    connect((UserLoginDialog*)dialog,
            SIGNAL(saveLoginDataSignal(QString, bool, QString)),
            config,
            SLOT(setLoginData(QString, bool, QString)));*/
    connect(DBcon, SIGNAL(loggedIn(int)), this, SLOT(logIn(int)));
}

void MainWindow::createSignInDialog()
{
    delete dialog;
    dialog = new UserSignInDialog(this);
    dialog->show();
    connect((UserSignInDialog*)dialog, SIGNAL(dataChecked(QString,QString)),
            DBcon, SLOT(createUser(QString,QString)));
    connect(DBcon, SIGNAL(userCreated()), dialog, SLOT(hide()));
    connect(DBcon, SIGNAL(userCreated()), this, SLOT(createLoginDialog()));
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

void MainWindow::displayError(const QString &message)
{
    statusMsg->setText("Error: "+message);
}

void MainWindow::clearErrorMessage()
{
    statusMsg->setText("");
}

void MainWindow::displayCurrentCellSettings(const QFont &font, 
                                            const QBrush &background, 
                                            const QBrush &foreground)
{
    fontButton->setText(font.family());
    colorPixmap->fill(background.color());
    backgroundColorButton->setIcon(QIcon(*colorPixmap));
    colorPixmap->fill(foreground.color());
    fontColorButton->setIcon(QIcon(*colorPixmap));
}

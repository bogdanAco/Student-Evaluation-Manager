#include "MainWindow.h"

MainWindow::MainWindow(const QString& title) : QMainWindow()
{
    this->setMinimumSize(550,600);
    this->setWindowTitle(title);

    dialog = 0;
    Spreadsheet = 0;
    connected = false;
    config = new CFGManager();
    DBcon = new DBManager(config);
    if(DBcon->connectDB(config->getDBUser(), config->getDBPassword()))
    {
        dialog = new UserLoginDialog(this);
        dialog->show();
        connect((UserLoginDialog*)dialog, SIGNAL(dataChecked(QString,QString)),
                DBcon, SLOT(login(QString,QString)));
        connect(DBcon, SIGNAL(loggedIn(int)), this, SLOT(logIn(int)));
    }
    else
    {
        connected = false;
        CreateErrorDialog("Not connected to database: please "
                          "check your database connection and"
                          " restart the application");
    }
    CreateActions();
    CreateMenus();
    CreateToolbars();
}

MainWindow::~MainWindow()
{
    delete Spreadsheet;
    delete dialog;
    delete DBcon;
    delete config;
    delete menuBar;
    delete fileToolBar;
    delete editToolBar;
    for (int i=0; i<fileActions.length(); i++)
        delete fileActions[i];
    for (int i=0; i<editActions.length(); i++)
        delete editActions[i];
}

void MainWindow::CreateMenus()
{
    menuBar = new QMenuBar(this);
    file = new QMenu("&File",menuBar);
    file->addActions(fileActions);
    file->insertSeparator(exportFileAction);
    edit = new QMenu("&Edit",menuBar);
    edit->addActions(editActions);
    menuBar->addMenu(file);
    menuBar->addMenu(edit);
    menuBar->setMinimumWidth(700);
    this->setMenuBar(menuBar);
}

void MainWindow::CreateActions()
{
    newFileAction = new QAction(QIcon("images/new.png"),
                                "&New table",this);
    newFileAction->setShortcut(QKeySequence::New);
    newFileAction->setStatusTip("Create new file");
    connect(newFileAction,SIGNAL(triggered()),this,SLOT(newFile()));
    fileActions << newFileAction;

    openFileAction = new QAction(QIcon("images/open.png"),
                                 "&Open table",this);
    openFileAction->setShortcut(QKeySequence::Open);
    openFileAction->setStatusTip("Open a file");
    connect(openFileAction,SIGNAL(triggered()),this,SLOT(openFile()));
    fileActions << openFileAction;

    closeFileAction = new QAction(QIcon("images/remove.png"),
                                  "&Close table",this);
    closeFileAction->setShortcut(QKeySequence::Close);
    closeFileAction->setStatusTip("Close the current file");
    connect(closeFileAction,SIGNAL(triggered()),this,SLOT(closeFile()));
    fileActions << closeFileAction;

    exportFileAction = new QAction(QIcon("images/save.png"),
                                   "&Export PDF",this);
    exportFileAction->setShortcut(QKeySequence::Save);
    exportFileAction->setStatusTip("Export the current spreadsheet to PDF");
    connect(exportFileAction,SIGNAL(triggered()),this,SLOT(exportFile()));
    exportFileAction->setDisabled(true);
    fileActions << exportFileAction;

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

    formulaAction = new QAction(QIcon("images/formula.png"), "Formula:", this);
    formulaAction->setStatusTip("Insert formula into cell");
    //connect
}

void MainWindow::CreateToolbars()
{
    fileToolBar = new QToolBar("&File", this);
    fileToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    fileToolBar->setMovable(false);
    this->addToolBar(fileToolBar);
    fileToolBar->addActions(fileActions);
    fileToolBar->insertSeparator(exportFileAction);

    editToolBar = new QToolBar("&Edit", this);
    editToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    editToolBar->setMovable(false);
    this->addToolBar(editToolBar);
    editToolBar->addActions(editActions);

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
    disconnect(dialog);
    dialog = new ErrorDialog(message, this);
    dialog->show();
}

void MainWindow::newFile()
{
    if (!connected)
        return;
    disconnect(dialog);
    dialog = new NewFileDialog(config, this);
    ((NewFileDialog*)dialog)->
            loadTreeData(DBcon->getFiles(), DBcon->getDirectories());
    connect((NewFileDialog*)dialog,
            SIGNAL(dataChecked(QString,int,int,QString)),
            DBcon, SLOT(createFile(QString,int,int,QString)));
    connect(DBcon, SIGNAL(tableCreated(QString,int,int)),
            this, SLOT(createNewFile(QString,int,int)));
    dialog->show();
}

void MainWindow::createNewFile(const QString &name, int columns, int rows)
{
    Spreadsheet = new SpreadSheet(rows, columns, this);
    Spreadsheet->setCellsSize(config->getCellsSize());
    setCentralWidget(Spreadsheet);
    DBcon->setCurrentSpreadSheet(Spreadsheet);

    connect(DBcon, SIGNAL(dataLoaded(QStringList)),
            Spreadsheet, SLOT(loadData(QStringList)));

    connect(Spreadsheet->getTimer(), SIGNAL(timeout()),
            DBcon, SLOT(getData()));
    connect(Spreadsheet, SIGNAL(modified(QString)),
            this, SLOT(writeToDB(QString)));

    setWindowTitle(QString("%1 - %2").arg(name, windowTitle()));

    exportFileAction->setEnabled(true);
    int size = columns * 110;
    if (size < QApplication::desktop()->width())
        this->setMinimumWidth(columns * 110);
}

void MainWindow::openFile()
{
    if (!connected)
        return;
    disconnect(dialog);
    dialog = new OpenFileDialog(this);
    ((OpenFileDialog*)dialog)->
            loadTreeData(DBcon->getFiles(), DBcon->getDirectories());
    connect((OpenFileDialog*)dialog,
            SIGNAL(dataChecked(QString,int,int,QString)),
            DBcon, SLOT(openFile(QString,int,int,QString)));
    connect(DBcon, SIGNAL(fileOpened(QString,int,int)),
            this, SLOT(openNewFile(QString,int,int)));
    dialog->show();
}

void MainWindow::openNewFile(const QString &name, int columns, int rows)
{
    Spreadsheet = new SpreadSheet(rows, columns, this);
    Spreadsheet->setCellsSize(config->getCellsSize());
    setCentralWidget(Spreadsheet);
    DBcon->setCurrentSpreadSheet(Spreadsheet);

    connect(DBcon, SIGNAL(dataLoaded(QStringList)),
            Spreadsheet, SLOT(loadData(QStringList)));
    DBcon->getData();

    connect(Spreadsheet->getTimer(), SIGNAL(timeout()),
            DBcon, SLOT(getData()));
    connect(Spreadsheet, SIGNAL(modified(QString)),
            this, SLOT(writeToDB(QString)));

    setWindowTitle(QString("%1 - %2").arg(name, windowTitle()));

    cellFormula->setEnabled(true);
    connect(cellFormula, SIGNAL(returnPressed()),
            this, SLOT(setFormula()));
    connect(Spreadsheet, SIGNAL(cellClicked(int,int)),
            this, SLOT(showFormula(int,int)));

    exportFileAction->setEnabled(true);
    int size = columns * 110;
    if (size < QApplication::desktop()->width())
        this->setMinimumWidth(columns * 110);
}

void MainWindow::closeFile()
{
    if (!connected || Spreadsheet == 0)
        return;
    disconnect(dialog);
    dialog = new Dialog("Close file",
             "Are you sure you want to close the file ?",
             this);
    connect(dialog, SIGNAL(okPressed()), this, SLOT(closeOpenedFile()));
    dialog->show();
}

void MainWindow::closeOpenedFile()
{
    setWindowTitle("Student Evaluation Manager");
    DBcon->removeCurrentData();
    delete Spreadsheet;
    Spreadsheet = 0;
    dialog->hide();
    exportFileAction->setDisabled(true);
    cellFormula->setText("");
    cellFormula->setDisabled(true);
}

void MainWindow::exportFile()
{
    QString file = QFileDialog::getSaveFileName(this);
    if (!Spreadsheet->printSpreadSheet(file))
    {
        disconnect(dialog);
        dialog = new ErrorDialog("PDF export error - "
                                 "append .pdf to file name", this);
        dialog->show();
    }
}

void MainWindow::cut()
{
    if (connected)
    {
        Spreadsheet->cut();
        cellFormula->setText("");
    }
}

void MainWindow::copy()
{
    if (connected)
        Spreadsheet->copy();
}

void MainWindow::paste()
{
    if (connected)
        Spreadsheet->paste();
}

void MainWindow::del()
{
    if (connected)
    {
        Spreadsheet->del();
        cellFormula->setText("");
    }
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

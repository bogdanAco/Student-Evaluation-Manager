#include "ConfigurationDialog.h"
#include "Security.h"

ConfigurationDialog::ConfigurationDialog(const CFGManager *cfg, QWidget *parent) :
        Dialog("Configuration", "", parent)
{
    this->cfg = cfg;
    setFixedSize(300, 480);
    text->hide();
    connect(cfg, SIGNAL(errorMessage(QString)),
            this, SLOT(showMessage(QString)));
    connect(this, SIGNAL(okPressed()), this, SLOT(accept()));
    connect(this, SIGNAL(okPressed()), cfg, SLOT(saveDoc()));
    connect(this, SIGNAL(cancelPressed()), cfg, SLOT(undoDoc()));
    tabs = new QTabWidget();
    mainLayout->addWidget(tabs, 4, 0, 1, 4);

    createDatabaseTab();
    createSpreadSheetTab();
    createSecurityTab();
}

ConfigurationDialog::~ConfigurationDialog()
{
    //database items
    delete dbType;
    delete dbServer;
    delete dbPort;
    delete dbName;
    delete dbUser;
    delete dbPass;
    delete dbRmFolderContent;
    delete dbBackupTables;
    delete databaseLayout;
    delete databaseTab;

    //spreadsheet items
    delete rowNo;
    delete columnNo;
    delete refreshTime;
    delete rowHeight;
    delete columnWidth;
    delete selectionMode;
    delete spreadSheetLayout;
    delete spreadSheetTab;

    //security items
    delete generateKeysButton;
    delete securityLayout;
    delete securityTab;

    delete tabs;
}

void ConfigurationDialog::createDatabaseTab()
{
    databaseTab = new QWidget();
    tabs->addTab(databaseTab, "Database");
    databaseLayout = new QVBoxLayout();
    databaseLayout->setAlignment(Qt::AlignTop);

    databaseLayout->addWidget(new QLabel("Database type:"));
    dbType = new QComboBox();
    dbType->addItem("MySQL");
    dbType->addItem("PostgreSQL");
    dbType->addItem("Oracle");
    QString databaseType = cfg->getDBType();
    if (databaseType == "QMYSQL")
        databaseType = "MySQL";
    else if (databaseType == "QPSQL")
        databaseType = "PostgreSQL";
    else if (databaseType == "QOCI")
        databaseType = "Oracle";
    dbType->setCurrentIndex(dbType->findText(databaseType));
    databaseLayout->addWidget(dbType, 0, Qt::AlignTop);
    connect(dbType, SIGNAL(currentIndexChanged(QString)),
            cfg, SLOT(setDBType(QString)));

    databaseLayout->addWidget(new QLabel("Server address:"));
    dbServer = new QLineEdit(cfg->getDBServer());
    databaseLayout->addWidget(dbServer);
    connect(dbServer, SIGNAL(textChanged(QString)),
            cfg, SLOT(setDBServer(QString)));

    databaseLayout->addWidget(new QLabel("Server port: (-1 for undefined)"));
    dbPort = new QSpinBox();
    dbPort->setRange(-1, 65535);
    dbPort->setValue(cfg->getDBPort());
    databaseLayout->addWidget(dbPort);
    connect(dbPort, SIGNAL(valueChanged(int)),
            cfg, SLOT(setDBPort(int)));

    databaseLayout->addWidget(new QLabel("Database:"));
    dbName = new QLineEdit(cfg->getDBName());
    databaseLayout->addWidget(dbName);
    connect(dbName, SIGNAL(textChanged(QString)),
            cfg, SLOT(setDBName(QString)));

    databaseLayout->addWidget(new QLabel("Username:"));
    dbUser = new QLineEdit(cfg->getDBUser());
    databaseLayout->addWidget(dbUser);
    connect(dbUser, SIGNAL(textChanged(QString)),
            cfg, SLOT(setDBUser(QString)));

    databaseLayout->addWidget(new QLabel("Password:"));
    dbPass = new QLineEdit(cfg->getDBPassword());
    dbPass->setEchoMode(QLineEdit::Password);
    databaseLayout->addWidget(dbPass);
    connect(dbPass, SIGNAL(textChanged(QString)),
            cfg, SLOT(setDBPassword(QString)));

    dbRmFolderContent = new QCheckBox("Remove contained tables "
                                                 "when removing folder");
    dbRmFolderContent->setChecked(cfg->removeChildren());
    databaseLayout->addWidget(dbRmFolderContent);
    connect(dbRmFolderContent, SIGNAL(toggled(bool)),
            cfg, SLOT(setRemoveChildren(bool)));

    dbBackupTables = new QCheckBox("Backup deleted tables");
    dbBackupTables->setChecked(cfg->backupTables());
    databaseLayout->addWidget(dbBackupTables);
    connect(dbBackupTables, SIGNAL(toggled(bool)),
            cfg, SLOT(setBackupTables(bool)));

    databaseLayout->addWidget(new QLabel("Remove backup after N days:"));
    dbRemoveBackup = new QSpinBox();
    dbRemoveBackup->setRange(0, INT_MAX);
    dbRemoveBackup->setValue(cfg->getBackupExpireDate());
    databaseLayout->addWidget(dbRemoveBackup);
    connect(dbRemoveBackup, SIGNAL(valueChanged(int)),
            cfg, SLOT(setBackupExpireDate(int)));

    databaseTab->setLayout(databaseLayout);
}

void ConfigurationDialog::createSpreadSheetTab()
{
    spreadSheetTab = new QWidget();
    tabs->addTab(spreadSheetTab, "SpreadSheet");
    spreadSheetLayout = new QVBoxLayout();
    spreadSheetLayout->setAlignment(Qt::AlignTop);

    spreadSheetLayout->addWidget(new QLabel("Row number:"));
    rowNo = new QSpinBox();
    rowNo->setRange(1,1000);
    rowNo->setValue(cfg->getRowCount());
    spreadSheetLayout->addWidget(rowNo);
    connect(rowNo, SIGNAL(valueChanged(int)),
            cfg, SLOT(setRowCount(int)));

    spreadSheetLayout->addWidget(new QLabel("Column number:"));
    columnNo = new QSpinBox();
    columnNo->setRange(1, 20);
    columnNo->setValue(cfg->getColumnCount());
    spreadSheetLayout->addWidget(columnNo);
    connect(columnNo, SIGNAL(valueChanged(int)),
            cfg, SLOT(setColumnCount(int)));

    spreadSheetLayout->addWidget(new QLabel("Spreadsheet refresh/"
                                            "update time:"));
    refreshTime = new QSpinBox();
    refreshTime->setRange(1, 10);
    refreshTime->setSuffix("s");
    refreshTime->setValue(cfg->getRefreshTime());
    spreadSheetLayout->addWidget(refreshTime);
    connect(refreshTime, SIGNAL(valueChanged(int)),
            cfg, SLOT(setRefreshTime(int)));

    QSize cellSize = cfg->getCellsSize();
    spreadSheetLayout->addWidget(new QLabel("Row height:"));
    rowHeight = new QSpinBox();
    rowHeight->setRange(20, 35);
    rowHeight->setSuffix("px");
    rowHeight->setValue(cellSize.height());
    spreadSheetLayout->addWidget(rowHeight);
    connect(rowHeight, SIGNAL(valueChanged(int)),
            cfg, SLOT(setRowHeight(int)));

    spreadSheetLayout->addWidget(new QLabel("Column width:"));
    columnWidth = new QSpinBox();
    columnWidth->setRange(100,200);
    columnWidth->setSuffix("px");
    columnWidth->setValue(cellSize.width());
    spreadSheetLayout->addWidget(columnWidth);
    connect(columnWidth, SIGNAL(valueChanged(int)),
            cfg, SLOT(setColumnWidth(int)));

    spreadSheetLayout->addWidget(new QLabel("Selection mode:"));
    selectionMode = new QComboBox();
    selectionMode->addItem("SingleSelection");
    selectionMode->addItem("ContiguousSelection");
    selectionMode->addItem("ExtendedSelection");
    selectionMode->addItem("MultiSelection");
    selectionMode->setCurrentIndex(
            selectionMode->findText(cfg->getSelectionModeText()));
    spreadSheetLayout->addWidget(selectionMode);
    connect(selectionMode, SIGNAL(currentIndexChanged(QString)),
            cfg, SLOT(setSelectionMode(QString)));

    spreadSheetTab->setLayout(spreadSheetLayout);
}

void ConfigurationDialog::createSecurityTab()
{
    securityTab = new QWidget();
    tabs->addTab(securityTab, "Security");
    securityLayout = new QVBoxLayout();
    securityLayout->setAlignment(Qt::AlignTop);

    securityLayout->addWidget(new QLabel("Generate new random keys:"));
    generateKeysButton = new QPushButton("Generate");
    generateKeysButton->setFixedWidth(150);
    securityLayout->addWidget(generateKeysButton);
    connect(generateKeysButton, SIGNAL(pressed()),
            this, SLOT(prepareKeysGeneration()));

    securityTab->setLayout(securityLayout);
}

void ConfigurationDialog::prepareKeysGeneration()
{
    UserLoginDialog *d = new UserLoginDialog(this);
    d->show();
    d->setWindowTitle("Passphrase generation");
    connect(d, SIGNAL(saveLoginDataSignal(QString,QString)),
            this, SLOT(generateKeys(QString,QString)));
    connect(d, SIGNAL(saveLoginDataSignal(QString,QString)),
            d, SLOT(hide()));
}

void ConfigurationDialog::generateKeys(const QString &name, const QString &pass)
{
    QString passphrase = Security::getHash(name+pass);
    QPair<QString,QString> keys = Security::generateKeyPair(passphrase);
    emit changeKeys(cfg->getKey(), keys.first, keys.second, passphrase);
}

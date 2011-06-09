#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include "Dialog.h"
#include "TableDialog.h"
#include "ConfigurationDialog.h"
#include "DBManager.h"
#include "CFGManager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString& title);
    void resetSize();
    ~MainWindow();
private:
    //Initialization functions
    void CreateMenus();
    void CreateActions();
    void CreateToolbars();
    //Dialogs
    Dialog *dialog;
    //SpreadSheet
    SpreadSheet *Spreadsheet;
    //Database Manager
    bool connected;
    DBManager *DBcon;
    //Configuration Manager
    CFGManager *config;
    //Menus
    QMenuBar *menuBar;
    QMenu *table;
    QMenu *edit;
    QMenu *application;
    //Toolbars
    QToolBar *tableToolBar;
    QToolBar *editToolBar;
    QToolBar *appToolBar;
    QToolBar *formulaToolBar;
    QLineEdit *cellFormula;
    //Table actions
    QList <QAction*> tableActions;
    QAction *newTableAction;
    QAction *openTableAction;
    QAction *closeTableAction;
    QAction *importTableAction;
    QAction *exportTableAction;
    //Edit actions
    QList <QAction*> editActions;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *deleteAction;
    QAction *addColumnAction;
    QAction *removeColumnAction;
    QAction *addRowAction;
    //Formula action
    QAction *formulaAction;
    //Application actions
    QList <QAction*> appActions;
    QAction *configureAction;
    QAction *loginAction;
    QAction *signinAction;
private slots:
    void CreateErrorDialog(const QString& message);
    void newTable();
    void createNewTable(const QString &name,
                       int columns, int rows);
    void openTable();
    void openNewTable(const QString &name,
                     int columns, int rows);
    void closeTable();
    void closeOpenedTable();
    void exportTable();
    void cut();
    void copy();
    void paste();
    void del();
    void writeToDB(const QString &cellData);
    void showFormula(int row, int column);
    void setFormula();
    void logIn(int uid);
    void createAddColumnsDialog();
    void createRemoveColumnsDialog();
    void createAddRowsDialog();
    void createConfigureAppDialog();
    void createImportDataDialog();
    void createFormulaDialog();
    void createLoginDialog();
    void createSignInDialog();
    void resizeWindow(int cells);
    void initializeDatabase();
};

#endif // MAINWINDOW_H

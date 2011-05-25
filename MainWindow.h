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
    void CreateErrorDialog(const QString& message);
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
    QMenu *insert;
    //Toolbars
    QToolBar *tableToolBar;
    QToolBar *editToolBar;
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
    QAction *configureAction;
    //Formula action
    QAction *formulaAction;
private slots:
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
    void addColumns();
    void removeColumns();
    void addRows();
    void configureApp();
    void importData();
    void formula();
    void resizeWindow(int cells);
};

#endif // MAINWINDOW_H

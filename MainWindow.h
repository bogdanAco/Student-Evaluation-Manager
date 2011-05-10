#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include "Dialog.h"
#include "DBManager.h"
#include "CFGManager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString& title);
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
    QMenu *file;
    QMenu *edit;
    QMenu *insert;
    //Toolbars
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *formulaToolBar;
    QLineEdit *cellFormula;
    //File actions
    QList <QAction*> fileActions;
    QAction *newFileAction;
    QAction *openFileAction;
    QAction *closeFileAction;
    QAction *exportFileAction;
    //Edit actions
    QList <QAction*> editActions;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *deleteAction;
    //Formula action
    QAction *formulaAction;
private slots:
    void newFile();
    void createNewFile(const QString &name,
                       int columns, int rows);
    void openFile();
    void openNewFile(const QString &name,
                     int columns, int rows);
    void closeFile();
    void closeOpenedFile();
    void exportFile();
    void cut();
    void copy();
    void paste();
    void del();
    void writeToDB(const QString &cellData);
    void showFormula(int row, int column);
    void setFormula();
    void logIn(int uid);
};

#endif // MAINWINDOW_H

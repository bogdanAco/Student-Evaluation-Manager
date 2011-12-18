#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QtCore>
#include <QtSql>
#include "SpreadSheet.h"
#include "Security.h"
#include "CFGManager.h"

class DBManager : public QObject
{
    Q_OBJECT
public:
    DBManager(const CFGManager *cfg);
    void setCurrentSpreadSheet(SpreadSheet *spreadsheet);
    void removeCurrentData();

    void initializeDatabase();
    bool connectDB(const QString& uname, const QString& pass);
    bool writeData(int line, int column, const QString& cell_data);
    int columnCount();
    QHash<QString, QString> getTables();
    QHash<QString, QString> getFolders();
    ~DBManager();

signals:
    void queryError(const QString &error);
    void rightsLoaded(const QList<int> columns);
    void rowsHeightLoaded(const QMap<int,int> size);
    void columnsWidthLoaded(const QMap<int,int> size);
    void columnsHeaderTextLoaded(const QMap<int, QString> data);
    void dataLoaded(const QMap<int, QString> &data);
    void dataLoaded(int row, int column, const QString &data);
    void givenDataLoaded(const QStringList &data);
    void tableCreated(const QString &data, int columns, int rows);
    void tableOpened(const QString &name, int columns, int rows);
    void loggedIn(int uid);
    void userCreated();
    void rowsAdded(int rows);
    void columnsAdded(int columns);
    void columnsRemoved(const QList<int> columns);
    void dataModified(const QHash<QString, QString> &tables,
                      const QHash<QString, QString> &folders);
    void message(const QString &msg);
    void initializeDatabaseRequest();
    void closeCurrentTable();

private:
    int current_user_id;
    int current_table_id;
    QSqlDatabase db;
    QSqlQuery *query;
    QString *current_table;
    SpreadSheet *spreadsheet;
    Security *security;
    const CFGManager *cfg;
    
    void deleteTable(int id);

public slots:
    void getData();
    void getData(const QString &table, int row, int column,
                 int destRow, int destCol);
    void getGivenData(int field, const QString &fieldVal,
                      const QString &table);
    void createTable(const QString &name, int columns,
                    int rows, const QString &folder);
    void openTable(const QString &name, int columns,
                  int rows, const QString &folder);

    void login(const QString& uname, const QString& pass);
    void createUser(const QString &uname, const QString &pass);

    void grantReadAccess(const QString &username);
    void grantWriteAccess(const QString &username, const QList<int> columns);

    void addColumns(int columns);
    void removeColumns(const QList <int> column_ids);
    void addRows(int rows);
    void setColumnWidth(int column, int oldSize, int newSize);
    void setColumnHeaderText(int column, const QString &text);
    void setRowHeight(int row, int oldSize, int newSize);

    void createFolder(const QString &name, const QString &parent);
    void removeFolder(const QString &name);
    void removeTable(const QString& name);

    void changeKey(const QString &oldPrivateKey,
                   const QString &publicKey,
                   const QString &privateKey,
                   const QString &passphrase);
};

#endif // DBMANAGER_H

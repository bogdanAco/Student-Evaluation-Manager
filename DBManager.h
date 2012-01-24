#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QtCore>
#include <QtSql>
#include "Security.h"
#include "CFGManager.h"

class SpreadSheet;

class DBManager : public QObject
{
    Q_OBJECT
public:
    DBManager(const CFGManager *cfg);
    void setCurrentSpreadSheet(SpreadSheet *spreadsheet);
    void removeCurrentData();

    void initializeDatabase(const QString &username, 
                            const QString &password);
    bool writeData(int line, int column, const QString& cell_data);
    int columnCount();
    int loadUsers();
    QHash<QString, QString> getTables();
    QHash<QString, QString> getFolders();
    ~DBManager();
    void disconnectDB();
    
signals:
    void queryError(const QString &error) const;
    void rightsLoaded(const QList<int> columns);
    void rowsHeightLoaded(const QMap<int,int> size);
    void columnsWidthLoaded(const QMap<int,int> size);
    void columnsHeaderTextLoaded(const QMap<int, QString> data);
    void dataLoaded(const QMap<int, QString> &data);
    void dataLoaded(int row, int column, const QString &data);
    void givenDataLoaded(const QMap<int, QString> &data);
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
    void initializeDatabaseRequest(const QString &username, 
                                   const QString &password,
                                   const QString &error);
    void closeCurrentTable();
    void usersLoaded(QList<QString> users);
    void rightsGranted();
    void setSpreadsheetSize(int rows, int columns);

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
    
    void login(const QString& uname, const QString& pass);
    void createUser(const QString &uname, const QString &pass);

public slots:
    void connectDB(const QString& uname, const QString& pass);
    
    void getData();
    QHash<QString,QString> getLinkData(const QHash<QString,QString> &matches) const;
    void getData(const QString &table);
    void createTable(const QString &name, int columns,
                    int rows, const QString &folder);
    void openTable(const QString &name, int columns,
                  int rows, const QString &folder);

    void grantReadAccess(const QString &username);
    void grantWriteAccess(const QString &username, const QList<int> columns);

    void addColumns(int columns);
    void removeColumns(const QList <int> column_ids);
    void addRows(int rows);
    void setColumnWidth(int column, int oldSize, int newSize);
    void setColumnHeaderText(int column, const QString &text);
    void setRowHeight(int row, int oldSize, int newSize);

    void createFolder(const QString &name, const QString &parent);
    bool removeFolder(const QString &name);
    bool removeTable(const QString& name);

    void changeKey(const QString &oldPrivateKey,
                   const QString &publicKey,
                   const QString &privateKey,
                   const QString &passphrase);
};

#endif // DBMANAGER_H

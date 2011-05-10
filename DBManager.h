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

    bool connectDB(const QString& uname, const QString& pass);
    bool writeData(int line, int column, const QString& cell_data);
    bool deleteFile(const QString& name, int client_id);
    int columnCount();
    QMap<QString, QString> getFiles();
    QMap<QString, QString> getDirectories();
    ~DBManager();

signals:
    void dataLoaded(const QStringList &data);
    void tableCreated(const QString &data, int columns, int rows);
    void fileOpened(const QString &name, int columns, int rows);
    void loggedIn(int uid);

private:
    int current_user_id;
    QSqlDatabase db;
    QSqlQuery *query;
    QString *current_table;
    SpreadSheet *spreadsheet;
    Security *security;

public slots:
    void getData();
    void createFile(const QString &name, int columns,
                    int rows, const QString &folder);
    void openFile(const QString &name, int columns,
                  int rows, const QString &folder);
    void login(const QString& uname, const QString& pass);
};

#endif // DBMANAGER_H

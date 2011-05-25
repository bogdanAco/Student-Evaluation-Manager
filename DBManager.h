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
    void setCurrentSpreadSheet(const SpreadSheet *spreadsheet);
    void removeCurrentData();

    bool connectDB(const QString& uname, const QString& pass);
    bool writeData(int line, int column, const QString& cell_data);
    int columnCount();
    QList<QPair<QString, QString> > getTables();
    QList<QPair<QString, QString> > getFolders();
    ~DBManager();

signals:
    void queryError(const QString &error);
    void dataLoaded(const QStringList &data);
    void givenDataLoaded(const QStringList &data);
    void tableCreated(const QString &data, int columns, int rows);
    void tableOpened(const QString &name, int columns, int rows);
    void loggedIn(int uid);
    void rowsAdded(int rows);
    void columnsAdded(int columns);
    void dataModified(QList<QPair<QString, QString> > tables,
                       QList<QPair<QString, QString> > folders);
    void message(const QString &msg);

private:
    int current_user_id;
    QSqlDatabase db;
    QSqlQuery *query;
    QString *current_table;
    const SpreadSheet *spreadsheet;
    const Security *security;
    const CFGManager *cfg;

public slots:
    void getData();
    void getGivenData(int field, const QString &fieldVal,
                      const QString &table);
    void createTable(const QString &name, int columns,
                    int rows, const QString &folder);
    void openTable(const QString &name, int columns,
                  int rows, const QString &folder);

    void login(const QString& uname, const QString& pass);

    void addColumns(int columns);
    void removeColumns(QList <int> column_ids);
    void addRows(int rows);

    void createFolder(const QString &name, const QString &parent);
    void removeFolder(const QString &name);
    void removeTable(const QString& name);

    void changeKey(const QString &oldKey, const QString &key);
    void changePKey(const QString &oldKey, const QString &pKey);
};

#endif // DBMANAGER_H

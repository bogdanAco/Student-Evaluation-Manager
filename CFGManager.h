#ifndef CFGMANAGER_H
#define CFGMANAGER_H

#include <QtXml>

class CFGManager : public QObject
{
    Q_OBJECT
public:
    CFGManager();
    ~CFGManager();
    //database configuration
    QString getDBType() const;
    QString getDBServer() const;
    int getDBPort() const;
    QString getDBName() const;
    QString getDBUser() const;
    QString getDBPassword() const;
    bool removeChildren() const;
    bool backupTables() const;
    int getBackupExpireDate() const;
    //spreadsheet configuration
    int getRowCount() const;
    int getColumnCount() const;
    int getRefreshTime() const;
    int getSelectionMode() const;
    QString getSelectionModeText() const;
    QSize getCellsSize() const;
    //security configuration
    QString getLoginUser() const;
    QString getLoginPass() const;
    bool getPassRemember() const;
    QString getKey() const;
    //personal key
    bool pKeyExists() const;
    QString getPKey() const;
    QString getPKey(const QString &user) const;

private:
    QDomDocument *domDoc;
    mutable QDomElement *root;
    QFile *XMLFile;
    QString *pKey;

public slots:
    void saveDoc() const;
    void saveUserKey(const QString &user,
                     const QString &key) const;
    void undoDoc() const;
    //database configuration
    void setDBType(const QString &type);
    void setDBServer(const QString &server);
    void setDBPort(int port);
    void setDBName(const QString &name);
    void setDBUser(const QString &user);
    void setDBPassword(const QString &pass);
    void setRemoveChildren(bool remove);
    void setBackupTables(bool backup);
    void setBackupExpireDate(int afterNDays);
    //spreadsheet configuration
    void setRowCount(int count);
    void setColumnCount(int count);
    void setRefreshTime(int seconds);
    void setSelectionMode(const QString &mode);
    void setCellsSize(const QSize &size);
    void setRowHeight(int height);
    void setColumnWidth(int width);
    //security configuration
    void setLoginData(const QString &name, bool rmb,
                      const QString &password);
    void setKey(const QString &key);
    void setPKey(const QString &key) const;

signals:
    void errorMessage(const QString &msg) const;
};

#endif // CFGMANAGER_H

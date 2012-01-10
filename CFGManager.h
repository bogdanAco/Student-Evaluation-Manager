#ifndef CFGMANAGER_H
#define CFGMANAGER_H

#include <QtXml>
#include <QAbstractItemView>

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
    QAbstractItemView::SelectionMode getSelectionMode() const;
    QString getSelectionModeText() const;
    QSize getCellsSize() const;
    //security configuration
    QString getKey() const;
    enum ErrorMessage { NoUser };

private:
    QDomDocument *domDoc;
    mutable QDomElement *root;
    mutable QDomElement *currentUser;
    QFile *XMLFile;

public slots:
    void emitErrorMessage(ErrorMessage m) const;
    void setCurrentUser(const QString &name) const;
    void saveDoc() const;
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
    void setKey(const QString &key) const;

signals:
    void errorMessage(const QString &msg) const;
};

#endif // CFGMANAGER_H

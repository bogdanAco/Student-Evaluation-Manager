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
    QString getDBName() const;
    QString getDBUser() const;
    QString getDBPassword() const;
    bool removeChildren() const;
    bool backupTables() const;
    //spreadsheet configuration
    int getRowCount() const;
    int getColumnCount() const;
    int getRefreshTime() const;
    int getSelectionMode() const;
    QString getSelectionModeText() const;
    QSize getCellsSize() const;
    //security configuration
    QString getKey() const;
    QString getAlgorithm() const;
    //personal key
    bool pKeyExists() const;
    QString getPKey() const;

private:
    QDomDocument *domDoc;
    mutable QDomElement *root;
    QFile *XMLFile;
    QString *pKey;

public slots:
    void saveDoc() const;
    void undoDoc() const;
    //database configuration
    void setDBType(const QString &type);
    void setDBServer(const QString &server);
    void setDBName(const QString &name);
    void setDBUser(const QString &user);
    void setDBPassword(const QString &pass);
    void setRemoveChildren(bool remove);
    void setBackupTables(bool backup);
    //spreadsheet configuration
    void setRowCount(int count);
    void setColumnCount(int count);
    void setRefreshTime(int seconds);
    void setSelectionMode(const QString &mode);
    void setCellsSize(const QSize &size);
    void setRowHeight(int height);
    void setColumnWidth(int width);
    //security configuration
    void setKey(const QString &key);
    void setPKey(const QString &key) const;
    void setAlgorithm(const QString &alg);

signals:
    void errorMessage(const QString &msg) const;
};

#endif // CFGMANAGER_H

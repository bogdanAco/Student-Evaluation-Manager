#ifndef CFGMANAGER_H
#define CFGMANAGER_H

#include <QtXml>

class CFGManager : public QObject
{
    Q_OBJECT
public:
    CFGManager();
    ~CFGManager();
    void saveDoc();
    //database configuration
    QString getDBType() const;
    QString getDBServer() const;
    QString getDBName() const;
    QString getDBUser() const;
    QString getDBPassword() const;
    //spreadsheet configuration
    int getRowCount() const;
    int getColumnCount() const;
    int getRefreshTime() const;
    int getSelectionMode() const;
    QSize getCellsSize() const;
    //security configuration
    QString getKey() const;
    QString getAlgorithm() const;

private:
    QDomDocument *domDoc;
    QDomElement *root;
    QFile *XMLFile;

public slots:
    //database configuration
    void setDBType(const QString &type);
    void setDBServer(const QString &server);
    void setDBName(const QString &name);
    void setDBUser(const QString &user);
    void setDBPassword(const QString &pass);
    //spreadsheet configuration
    void setRowCount(int count);
    void setColumnCount(int count);
    void setRefreshTime(int seconds);
    void setSelectionMode(const QString &mode);
    void setCellsSize(const QSize &size);
    //security configuration
    void setKey(const QString &key);
    void setAlgorithm(const QString &alg);
};

#endif // CFGMANAGER_H

#ifndef CONFIGURATIONDIALOG_H
#define CONFIGURATIONDIALOG_H

#include "Dialog.h"

class ConfigurationDialog : public Dialog
{
    Q_OBJECT
public:
    ConfigurationDialog(const CFGManager *cfg, QWidget *parent = 0);
    ~ConfigurationDialog();

private:
    void createDatabaseTab();
    void createSpreadSheetTab();
    void createSecurityTab();

    QTabWidget *tabs;
    const CFGManager *cfg;

    //database items
    QWidget *databaseTab;
    QVBoxLayout *databaseLayout;
    QComboBox *dbType;
    QLineEdit *dbServer;
    QSpinBox *dbPort;
    QLineEdit *dbName;
    QLineEdit *dbUser;
    QLineEdit *dbPass;
    QCheckBox *dbRmFolderContent;
    QCheckBox *dbBackupTables;
    QSpinBox *dbRemoveBackup;

    //spreadsheet items
    QWidget *spreadSheetTab;
    QVBoxLayout *spreadSheetLayout;
    QSpinBox *rowNo;
    QSpinBox *columnNo;
    QSpinBox *refreshTime;
    QSpinBox *rowHeight;
    QSpinBox *columnWidth;
    QComboBox *selectionMode;

    //security items
    QWidget *securityTab;
    QVBoxLayout *securityLayout;
    QLineEdit *key;
    QLineEdit *pKey;
    QComboBox *algorithm;
    QString *initialKey;
    QString *initialPKey;
    QPushButton *generateKeyButton;
    QPushButton *generatePKeyButton;

private slots:
    void emitKeyChanged();
    void emitPKeyChanged();
    void generateKey();
    void generatePKey();

signals:
    void changeKey(const QString &oldKey,
                   const QString &key);
    void changePKey(const QString &oldKey,
                    const QString &pKey);
};

#endif // CONFIGURATIONDIALOG_H

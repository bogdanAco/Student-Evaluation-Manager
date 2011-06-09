#include "DBManager.h"

DBManager::DBManager(const CFGManager *cfg)
{
    this->cfg = cfg;

    db = QSqlDatabase::addDatabase(this->cfg->getDBType());
    db.setHostName(this->cfg->getDBServer());
    db.setDatabaseName(this->cfg->getDBName());
    db.setPort(this->cfg->getDBPort());

    current_table = new QString();
    current_user_id = -1;

    security = new Security(this->cfg->getKey());
}

void DBManager::setCurrentSpreadSheet(const SpreadSheet *spreadsheet)
{
    this->spreadsheet = spreadsheet;

    connect(this, SIGNAL(dataLoaded(QStringList)),
            this->spreadsheet, SLOT(loadData(QStringList)));
    connect(this->spreadsheet, SIGNAL(getLink(QString,int,int,int,int)),
            this, SLOT(getData(QString,int,int,int,int)));
    connect(this, SIGNAL(dataLoaded(int,int,QString)),
            this->spreadsheet, SLOT(setFormula(int,int,QString)));
    getData();

    connect(this->spreadsheet->getTimer(), SIGNAL(timeout()),
            this, SLOT(getData()));
    connect(this, SIGNAL(columnsAdded(int)),
            this->spreadsheet, SLOT(addColumns(int)));
    connect(this, SIGNAL(rowsAdded(int)),
            this->spreadsheet, SLOT(addRows(int)));
    connect(this, SIGNAL(columnsRemoved(QList<int>)),
            this->spreadsheet, SLOT(removeColumns(QList<int>)));
}

void DBManager::removeCurrentData()
{
    current_table->clear();
}

void DBManager::initializeDatabase()
{
    db.setDatabaseName("");
    if (!db.open())
    {
        emit queryError("Unable to initialize database");
        return;
    }
    query = new QSqlQuery(db);
        if (!query->exec(QString("CREATE DATABASE %1").arg(cfg->getDBName())))
    {
        emit queryError("Unable to create database");
        return;
    }
    db.close();
    db.setDatabaseName(cfg->getDBName());
    if (!db.open())
    {
        emit queryError("Unable to open the created database");
        return;
    }

    QFile f("tables.sql");
    if (!f.open(QIODevice::ReadOnly | QFile::Text))
    {
        emit queryError("Unable to find the sql file");
        db.close();
        return;
    }
    QString data = QString(f.readAll());
    f.close();
    QStringList queries = data.split(';');
    for (int i=0; i<queries.length()-1; i++)
    {
        if (!query->exec(queries.at(i)))
        {
            emit queryError("Unable to create tables");
            db.close();
            return;
        }
    }
}

bool DBManager::connectDB(const QString &uname, const QString &pass)
{
    db.setUserName(uname);
    db.setPassword(pass);
    if (!db.open())
    {
        QString error = QString();
        switch (db.lastError().number())
        {
        case 1044:
            error = "invalid username";
            break;
        case 1045:
            error = "invalid username or password";
            break;
        case 1049:
            error = db.lastError().databaseText();
            emit initializeDatabaseRequest();
            return false;
        case 2003:
            error = "invalid server port";
            break;
        case 2005:
            error = "invalid server type or address";
            break;
        default:
            error = db.lastError().databaseText();
            break;
        }

        emit queryError(QString("Not connected to the database:\n").
                        append(error));
        return false;
    }
    else
    {
        query = new QSqlQuery(db);
        return true;
    }
}

void DBManager::login(const QString& uname, const QString& pass)
{
    if (!db.isOpen())
    {
        emit queryError("Not connected to the database\n"
                        "Please restart the application");
        return;
    }

    if (!cfg->pKeyExists())
        cfg->setPKey(security->generateKey());
    QString username = security->encryptData(uname, cfg->getPKey());
    QString password = security->encryptData(pass, cfg->getPKey());
    query->prepare("SELECT user_id, user_name, passwd "
                   "FROM users "
                   "WHERE user_name=:usr AND passwd=:pass");
    query->bindValue(":usr",username);
    query->bindValue(":pass",password);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    if (query->size() <= 0)
    {
        emit loggedIn(-1);
        return;
    }

    while (query->next())
        current_user_id = query->value(0).toInt();

    query->prepare("SELECT backup_name "
                   "FROM backup "
                   "WHERE owner=:owner "
                   "AND expire_date<:today");
    query->bindValue(":owner", current_user_id);
    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    if (db.driverName() == "QOCI")
        query->bindValue(":today", QString("TO_DATE('%1','YYYY-MM-DD')").
                                        arg(today));
    else
        query->bindValue(":today", today);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }

    QStringList backup_tables = QStringList();
    while (query->next())
        backup_tables.append(query->value(0).toString());
    for (int i=0; i<backup_tables.length(); i++)
    {
        query->exec(QString("DROP TABLE %1").arg(backup_tables.at(i)));
        query->prepare("DELETE FROM backup WHERE backup_name=:table");
        query->bindValue(":table", backup_tables.at(i));
        query->exec();
    }

    emit loggedIn(current_user_id);
}

void DBManager::createUser(const QString &uname, const QString &pass)
{
    QString username = security->encryptData(uname, cfg->getPKey());
    QString password = security->encryptData(pass, cfg->getPKey());
    query->prepare("SELECT * "
                   "FROM users "
                   "WHERE user_name=:usr");
    query->bindValue(":usr",username);
    if (query->size() > 0)
    {
        emit queryError("User already exists");
        return;
    }

    if (!query->exec("SELECT val FROM current_ids WHERE type='user'"))
    {
        emit queryError("Please check your database connection");
        return;
    }

    int uid = -1;
    while (query->next())
        uid = query->value(0).toInt();

    query->prepare("INSERT INTO users "
                   "VALUES (:uid, :usr, :pass)");
    query->bindValue(":uid", uid+1);
    query->bindValue(":usr", username);
    query->bindValue(":pass", password);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }

    if (!query->exec("UPDATE current_ids "
                     "SET val=val+1 "
                     "WHERE type='user'"))
    {
        emit queryError("Please check your database connection");
        return;
    }

    emit userCreated();
}

bool DBManager::writeData(int line, int column, const QString& cell_data)
{
    QString dataToWrite;
    (cell_data == "")?(dataToWrite = ""):
            (dataToWrite = security->encryptData(cell_data));

    query->prepare(QString("SELECT row_index FROM %1 "
                           "WHERE row_index = %2")
                           .arg(*current_table).arg(line));
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return false;
    }
    int size = query->size();
    if (size == 0)
    {
        QString timestamp = QDate::currentDate().toString("dd/MM/yyyy");
        timestamp.append("&");
        timestamp.append(QTime::currentTime().toString("hh:mm:ss:zzz"));

        query->prepare(QString("INSERT INTO %1"
                       "(row_index, row_timestamp, %2)"
                       " VALUES (:rowIndex, :timestamp, :fieldVal)").
                       arg(*current_table).arg(QString("field%1").
                       arg(column)));
        query->bindValue(":rowIndex", line);
        query->bindValue(":timestamp", timestamp);
        query->bindValue(":fieldVal", dataToWrite);
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return false;
        }

        spreadsheet->addTimestamp(timestamp);
        return true;
    }
    else if (size > 0)
    {
        QString timestamp = QDate::currentDate().toString("dd/MM/yyyy");
        timestamp.append("&");
        timestamp.append(QTime::currentTime().toString("hh:mm:ss:zzz"));

        query->prepare(QString("UPDATE %1 "
                               "SET row_timestamp = :timestamp, "
                               "%2 = :fieldVal "
                               "WHERE row_index = :rowIndex").
                                arg(*current_table).
                                arg(QString("field%1").arg(column)));
        query->bindValue(":fieldVal", dataToWrite);
        query->bindValue(":timestamp", timestamp);
        query->bindValue(":rowIndex", line);
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return false;
        }

        spreadsheet->replaceTimestamp(line, timestamp);
        return true;
    }
    emit queryError("Please check your database connection");
    return false;
}

void DBManager::getData()
{
    QString aux = QString("SELECT * "
                          "FROM %1").arg(*current_table);
    int timestampCount = spreadsheet->timestampCount();
    QString aux2 = "";
    if (timestampCount > 0)
    {
        aux2.append(" WHERE row_timestamp NOT IN (");
        for (int i=0; i<timestampCount; i++)
        {
            aux2.append("'");
            aux2.append(spreadsheet->getTimestamp(i));
            aux2.append("', ");
        }
        aux2.remove(aux2.length()-2, 2);
        aux2.append(")");
    }
    aux.append(aux2);
    aux.append(" ORDER BY row_index");

    if (!query->exec(aux))
    {
        emit queryError("Please check your database connection");
        return;
    }

    QSqlRecord record = query->record();
    int columns = record.count();
    if (columns == 0)
        return;

    bool add = false;
    if (timestampCount == 0)
        add = true;
    QStringList current_data = QStringList();
    while (query->next())
    {
        aux.clear();
        for (int i=2; i<columns; i++)
        {
            QString data = query->value(i).toString();
            QString decryptedData;
            (data == "")?(decryptedData = ""):
                    (decryptedData = security->decryptData(data));
            aux.append(decryptedData+"\n");
        }
        aux.remove(aux.length()-1, 1);
        current_data.append(aux);
        if (add)
            spreadsheet->addTimestamp(query->value(record.
                         indexOf("row_timestamp")).toString());
    }
    emit dataLoaded(current_data);
}

void DBManager::getData(const QString &table, int row, int column,
                        int destRow, int destCol)
{
    QString aux = QString("SELECT table_name "
                          "FROM files "
                          "WHERE file_name='%1'").arg(table);
    if (!query->exec(aux))
    {
        emit queryError("Please check your database connection");
        return;
    }
    while (query->next())
        aux = query->value(0).toString();

    query->prepare(QString("SELECT field%1 FROM %2 "
                           "WHERE row_index=%3").
                   arg(column).arg(aux).arg(row));
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    if (query->size() != 1)
        return;
    while (query->next())
        aux = query->value(0).toString();

    QString data = QString("=link(%1;%2;%3;%4)").
                   arg(table).arg(row).arg(column).
                   arg(security->decryptData(aux));
    emit dataLoaded(destRow, destCol, data);
}

void DBManager::getGivenData(int field, const QString &fieldVal,
                             const QString &table)
{
    if (field < 0 || fieldVal.length() == 0 || table.length() == 0)
        return;

    QString aux = QString("SELECT table_name "
                          "FROM files "
                          "WHERE file_name='%1'").arg(table);
    if (!query->exec(aux))
    {
        emit queryError("Please check your database connection");
        return;
    }

    while (query->next())
        aux = query->value(0).toString();
    query->prepare(QString("SELECT * FROM %1 "
                           "WHERE field%2=:val").
                   arg(aux).arg(field));
    query->bindValue(":val", security->encryptData(fieldVal));
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }

    if (query->size() > 1)
        return;
    QSqlRecord record = query->record();
    int columns = record.count();
    if (columns == 0)
        return;

    int row = 0;
    QStringList current_data = QStringList();
    while (query->next())
    {
        for (int i=2; i<columns; i++)
        {
            QString decryptedData;
            QString data = query->value(i).toString();
            (data == "")?(decryptedData = ""):
                    (decryptedData = security->decryptData(data));

            current_data.append(decryptedData);
        }
        row = query->value(record.indexOf("row_index")).toInt();
    }
    emit givenDataLoaded(current_data);
}

void DBManager::createTable(const QString &name, int columns,
                           int rows, const QString &folder)
{
    QSqlRecord record;
    if (!query->exec("SELECT * "
                    "FROM current_ids "
                    "WHERE type='file'"))
    {
        emit queryError("Please check your database connection");
        return;
    }
    record = query->record();
    int colIndex = record.indexOf("val");
    int current_index = 0;
    while (query->next())
        current_index = query->value(colIndex).toInt();

    QString tableName = QString("table%1").arg(current_index);
    QString aux = QString("CREATE TABLE %1 (row_index INT NOT NULL, "
                          "row_timestamp VARCHAR(25) NOT NULL, ").arg(tableName);
    for (int i=0; i<columns; i++)
        aux.append(QString("field%1 VARCHAR(512), ").arg(i));
    aux.append("CONSTRAINT row_index_pk "
               "PRIMARY KEY (row_index) )");
    if (!query->exec(aux))
    {
        emit queryError("Please check your database connection");
        return;
    }

    query->prepare("INSERT INTO files "
                   "VALUES (:id, :tableName, :fileName, :owner, :rows, "
                   "(SELECT folder_id FROM folders WHERE folder_name = :folder) )");
    query->bindValue(":id", current_index);
    query->bindValue(":tableName", tableName);
    query->bindValue(":fileName", name);
    query->bindValue(":owner", current_user_id);
    query->bindValue(":rows", rows);
    query->bindValue(":folder", folder);
    if (!query->exec())
    {
        query->exec(QString("DROP TABLE %1").arg(tableName));
        return;
    }

    query->prepare("UPDATE current_ids "
                   "SET val=:value "
                   "WHERE type='file'");
    query->bindValue(":value", current_index+1);
    if (!query->exec())
    {
        query->exec(QString("DROP TABLE %1").arg(tableName));
        query->prepare("DELETE FROM files WHERE file_id=:fileID");
        query->bindValue(":fileID", current_index);
        return;
    }
    current_table = new QString(tableName);
    emit tableCreated(tableName, columns, rows);
}

void DBManager::openTable(const QString &name, int columns,
              int rows, const QString &folder)
{
    query->prepare("SELECT table_name, row_count FROM files "
                   "WHERE file_name = :name");
    query->bindValue(":name", name);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    int row_count = 0;
    while (query->next())
    {
        current_table = new QString(query->value(0).toString());
        row_count = query->value(1).toInt();
    }

    if (!query->exec(QString("SELECT * FROM %1").arg(*current_table)))
        return;
    QSqlRecord record = query->record();
    int colCount = record.count() - 2;
    emit tableOpened(name, colCount, row_count);
}

int DBManager::columnCount()
{
    if (!query->exec(QString("SELECT * FROM %1").arg(*current_table)))
        return 0;
    QSqlRecord record = query->record();
    return record.count()-2;
}

QList<QPair<QString, QString> > DBManager::getTables()
{
    QList<QPair<QString, QString> > tables = QList<QPair<QString, QString> >();
    if (!query->exec("SELECT fi.file_name, fo.folder_name "
                     "FROM files fi, folders fo "
                     "WHERE fi.folder = fo.folder_id"))
        return tables;
    while (query->next())
        tables.append(QPair<QString,QString>(query->value(0).toString(),
                                             query->value(1).toString()));
    return tables;
}

QList<QPair<QString, QString> > DBManager::getFolders()
{
    QList<QPair<QString, QString> > dirs = QList<QPair<QString, QString> >();
    if (!query->exec("SELECT folder_name, folder_parent "
                     "FROM folders ORDER BY folder_id ASC"))
        return dirs;
    while (query->next())
        dirs.append(QPair<QString,QString>(query->value(0).toString(),
                                           query->value(1).toString()));
    return dirs;
}

DBManager::~DBManager()
{
    if (db.isOpen())
        delete query;
    db.close();
    db.removeDatabase("mng_users");
    delete security;
    delete current_table;
}

void DBManager::addColumns(int columns)
{
    int fieldCount = columnCount();
    for (int i=0; i<columns; i++)
    {
        QString q = QString("ALTER TABLE %1 ADD COLUMN field%2 VARCHAR(512)").
                    arg(*current_table).arg(fieldCount+i);
        if (!query->exec(q))
        {
            emit queryError("Please check your database connection");
            return;
        }
    }
    emit columnsAdded(columns);
}

void DBManager::removeColumns(const QList <int> column_ids)
{
    QString q;
    for (int i=0; i<column_ids.length(); i++)
    {
        q = QString("ALTER TABLE %1 DROP COLUMN field%2").
                    arg(*current_table).arg(column_ids.at(i));
        if (!query->exec(q))
        {
            emit queryError("Please check your database connection1");
            return;
        }
    }

    q = QString("ALTER TABLE %1 RENAME TO %1_aux").arg(*current_table);
    if (!query->exec(q))
    {
        emit queryError("Please check your database connection2");
        return;
    }

    int newFieldCount = spreadsheet->columnCount() - column_ids.length();
    q = QString("CREATE TABLE %1 (row_index INT NOT NULL, "
                "row_timestamp VARCHAR(25) NOT NULL, ").arg(*current_table);
    for (int i=0; i<newFieldCount; i++)
        q.append(QString("field%1 VARCHAR(512), ").arg(i));
    q.append("CONSTRAINT row_index_pk PRIMARY KEY (row_index) )");
    if (!query->exec(q))
    {
        emit queryError("Please check your database connection3");
        return;
    }

    if (!query->exec(QString("INSERT INTO %1 "
                             "(SELECT * FROM %1_aux)").
                                arg(*current_table)))
    {
        emit queryError("Please check your database connection4");
        return;
    }
    if (!query->exec(QString("DROP TABLE %1_aux").arg(*current_table)))
    {
        emit queryError("Please check your database connection5");
        return;
    }

    emit columnsRemoved(column_ids);
}

void DBManager::addRows(int rows)
{
    query->prepare("UPDATE files SET row_count=row_count+:count");
    query->bindValue(":count", rows);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    emit rowsAdded(rows);
}

void DBManager::createFolder(const QString &name, const QString &parent)
{
    if (!query->exec("SELECT val FROM current_ids WHERE type='folder'"))
    {
        emit queryError("Please check your database connection");
        return;
    }
    int current_index = 0;
    while (query->next())
        current_index = query->value(0).toInt();

    query->prepare("INSERT INTO folders VALUES (:index, :fname, :fparent)");
    query->bindValue(":index", current_index);
    query->bindValue(":fname", name);
    query->bindValue(":fparent", parent);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }

    query->prepare("UPDATE current_ids SET val=:value WHERE type='folder'");
    query->bindValue(":value", current_index+1);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }

    emit dataModified(getTables(), getFolders());
}

void DBManager::removeFolder(const QString &name)
{
    bool removeChildren = cfg->removeChildren();

    query->prepare("SELECT folder_id FROM folders "
                   "WHERE folder_name=:fname");
    query->bindValue(":fname", name);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }

    int folder_index = 0;
    while (query->next())
        folder_index = query->value(0).toInt();

    query->prepare("DELETE FROM folders "
                   "WHERE folder_id=:fid");
    query->bindValue(":fid", folder_index);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }

    query->prepare("DELETE FROM folders "
                   "WHERE folder_parent=:fname");
    query->bindValue(":fname", name);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }

    query->prepare("UPDATE folders "
                   "SET folder_id=folder_id-1 "
                   "WHERE folder_id>:fid");
    query->bindValue(":fid", folder_index);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }

    if (!query->exec("UPDATE current_ids "
                     "SET val=val-1 "
                     "WHERE type='folder'"))
    {
        emit queryError("Please check your database connection");
        return;
    }

    if (removeChildren)
    {
        QStringList tables = QStringList();
        query->prepare("SELECT table_name FROM files WHERE folder=:fid");
        query->bindValue(":fid", folder_index);
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return;
        }

        while (query->next())
            tables.append(query->value(0).toString());

        bool backupTables = cfg->backupTables();
        QString q = "";
        if (backupTables)
            q = QString("ALTER TABLE %1 RENAME TO %1_temp");
        else
            q = QString("DROP TABLE %1");
        for (int i=0; i<tables.length(); i++)
            if (!query->exec(q.arg(tables.at(i))))
                return;

        query->prepare("DELETE FROM files WHERE folder=:fid");
        query->bindValue(":fid", folder_index);
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return;
        }
    }
    else
    {
        query->prepare("UPDATE files SET folder=0 WHERE folder=:fid");
        query->bindValue(":fid", folder_index);
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return;
        }
    }

    emit dataModified(getTables(), getFolders());
}

void DBManager::removeTable(const QString& name)
{
    query->prepare("SELECT file_id, table_name, owner "
                   "FROM files "
                   "WHERE file_name=:fileName");
    query->bindValue(":fileName", name);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }

    QSqlRecord record = query->record();
    if (record.count() == 0)
        return;

    int file_id = 0;
    int owner = 0;
    QString tableName = "";
    while (query->next())
    {
        file_id = query->value(record.indexOf("file_id")).toInt();
        tableName = query->value(record.indexOf("table_name")).toString();
        owner = query->value(record.indexOf("owner")).toInt();
    }
    if (owner != current_user_id)
        return;

    bool backupTables = cfg->backupTables();
    QString q = QString();
    if (backupTables)
    {
        QDate expireDate;
        int daysToKeep = cfg->getBackupExpireDate();
        if (daysToKeep == 0)
            expireDate = QDate::fromString("9999-12-31", "yyyy-MM-dd");
        else
            expireDate = QDate::currentDate().addDays(daysToKeep);
        QString backupTableName = QString("%1_temp").
                                  arg(tableName).
                                  append(expireDate.toString("yyyyMMdd"));
        q = QString("ALTER TABLE %1 RENAME TO %2").
            arg(tableName).arg(backupTableName);
        if (!query->exec(q))
        {
            emit queryError("Please check your database connection");
            return;
        }

        query->prepare("INSERT INTO backup VALUES (:tableName, :date, :uid)");
        query->bindValue(":tableName", backupTableName);
        query->bindValue(":date", expireDate.toString("yyyy-MM-dd"));
        query->bindValue(":uid", current_user_id);
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return;
        }
    }
    else
    {
        q = QString("DROP TABLE %1");
        if (!query->exec(q.arg(tableName)))
        {
            emit queryError("Please check your database connection");
            return;
        }
    }

    query->prepare("DELETE FROM files WHERE file_id=:f_id");
    query->bindValue(":f_id", file_id);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }

    query->prepare("UPDATE files "
                   "SET file_id=file_id-1 "
                   "WHERE file_id>:f_id");
    query->bindValue(":f_id", file_id);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }

    if (!query->exec("UPDATE current_ids "
                     "SET val=val-1 "
                     "WHERE type='file'"))
    {
        emit queryError("Please check your database connection");
        return;
    }

    emit dataModified(getTables(), getFolders());
}

void DBManager::changeKey(const QString &oldKey,
                          const QString &key)
{
    if (key.length() != 32)
    {
        emit message("Invalid key size (must be 32)");
        return;
    }

    if (!query->exec("SELECT table_name FROM files"))
    {
        emit queryError("Please check your database connection");
        return;
    }

    QStringList tables = QStringList();
    while (query->next())
        tables.append(query->value(0).toString());

    QList<QStringList> encryptedData = QList<QStringList>();
    for (int i=0; i<tables.length(); i++)
    {
        if (!query->exec(QString("SELECT * FROM %1").
                       arg(tables.at(i))))
        {
            emit queryError("Please check your database connection");
            return;
        }

        int columns = query->record().count();
        QStringList encryptedRow = QStringList();
        while (query->next())
        {
            encryptedRow.clear();
            for (int c=2; c<columns; c++)
            {
                QString data = query->value(c).toString();
                if (data == "")
                    encryptedRow.append("");
                else
                {
                    QString decrypted = security->decryptData(data, oldKey);
                    encryptedRow.append(security->encryptData(decrypted, key));
                }
            }
            encryptedData.append(encryptedRow);
        }

        if (!query->exec(QString("DELETE FROM %1").arg(tables.at(i))))
        {
            emit queryError("Please check your database connection");
            return;
        }

        for (int r=0; r<encryptedData.length(); r++)
        {
            QString timestamp = QDate::currentDate().toString("dd/MM/yyyy");
            timestamp.append("&");
            timestamp.append(QTime::currentTime().toString("hh:mm:ss:zzz"));

            QString aux = QString("INSERT INTO %1 VALUES (%2, '%3', '").
                          arg(tables.at(i)).arg(r).arg(timestamp);
            for (int c=0; c<encryptedRow.length(); c++)
                aux.append(encryptedData.at(r).at(c)).append("', '");
            aux.remove(aux.length()-3, 3);
            aux.append(")");
            if (!query->exec(aux))
            {
                emit queryError("Please check your database connection");
                return;
            }
        }
    }

    security->loadKey(key);
}

void DBManager::changePKey(const QString &oldKey,
                           const QString &pKey)
{
    if (pKey.length() != 32)
    {
        emit message("Invalid key size (must be 32)");
        return;
    }

    query->prepare("SELECT user_name, passwd "
                   "FROM users "
                   "WHERE user_id=:uid");
    query->bindValue(":user_id", current_user_id);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    if (query->size() <= 0)
        return;

    QString username, password;
    while (query->next())
    {
        username = query->value(0).toString();
        password = query->value(1).toString();
    }

    username = security->decryptData(username, oldKey);
    password = security->decryptData(password, oldKey);

    username = security->encryptData(username, pKey);
    password = security->encryptData(password, pKey);

    query->prepare("UPDATE users "
                   "SET user_name=:usr, passwd=:pass "
                   "WHERE user_id=:uid");
    query->bindValue(":usr", username);
    query->bindValue(":pass", password);
    query->bindValue(":user_id", current_user_id);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }

    emit message("");
}

#include "DBManager.h"
#include "SpreadSheet.h"

DBManager::DBManager(const CFGManager *cfg)
{
    this->cfg = cfg;

    if (!QSqlDatabase::drivers().contains(cfg->getDBType()))
    {
        emit queryError("Database driver not found. Copy "
                        "the driver into sqldrivers directory "
                        "and restart the application");
        return;
    }
    db = QSqlDatabase::addDatabase(this->cfg->getDBType());
    db.setHostName(this->cfg->getDBServer());
    db.setPort(this->cfg->getDBPort());
    db.setDatabaseName(this->cfg->getDBName());

    current_table = new QString();
    current_user_id = -1;
    current_table_id = -1;

    security = new Security();
}

void DBManager::setCurrentSpreadSheet(SpreadSheet *spreadsheet)
{
    this->spreadsheet = spreadsheet;

    connect(this, SIGNAL(dataLoaded(QMap<int,QString>)),
            this->spreadsheet, SLOT(loadData(QMap<int,QString>)));
    connect(this, SIGNAL(rightsLoaded(QList<int>)),
            this->spreadsheet, SLOT(setRights(QList<int>)));
    connect(this, SIGNAL(rowsHeightLoaded(QMap<int,int>)),
            this->spreadsheet, SLOT(setRowsSize(QMap<int,int>)));
    connect(this, SIGNAL(columnsWidthLoaded(QMap<int,int>)),
            this->spreadsheet, SLOT(setColumnsSize(QMap<int,int>)));
    connect(this, SIGNAL(columnsHeaderTextLoaded(QMap<int,QString>)),
            spreadsheet, SLOT(setColumnsHeaderText(QMap<int,QString>)));
    getData();

    connect(this->spreadsheet->getTimer(), SIGNAL(timeout()),
            this, SLOT(getData()));
    connect(this, SIGNAL(columnsAdded(int)),
            this->spreadsheet, SLOT(addColumns(int)));
    connect(this, SIGNAL(rowsAdded(int)),
            this->spreadsheet, SLOT(addRows(int)));
    connect(this, SIGNAL(columnsRemoved(QList<int>)),
            this->spreadsheet, SLOT(removeColumns(QList<int>)));
    connect(this->spreadsheet, SIGNAL(columnResize(int,int,int)),
            this, SLOT(setColumnWidth(int,int,int)));
    connect(this->spreadsheet, SIGNAL(rowResize(int,int,int)),
            this, SLOT(setRowHeight(int,int,int)));
}

void DBManager::removeCurrentData()
{
    current_table->clear();
    current_table_id = -1;
}

//OK, TESTED, WORKING
void DBManager::initializeDatabase(const QString &username, 
                                   const QString &password)
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
        emit queryError("Unable to find the SQL file");
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
            emit queryError("Unable to create the SQL tables");
            db.close();
            return;
        }
    }
    createUser(username, password);
}

//OK TESTED WORKING
void DBManager::grantReadAccess(const QString &username)
{
    query->prepare("SELECT user_id, public_key "
                   "FROM users "
                   "WHERE user_name=:name");
    query->bindValue(":name", username);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    if (query->size() == 0)
    {
        emit queryError("User does not exist");
        return;
    }

    int uid = -1;
    QString pubkey = "";
    while (query->next())
    {
        uid = query->value(0).toInt();
        pubkey = query->value(1).toString();
    }
    
    query->prepare("SELECT access_key "
                   "FROM access_keys "
                   "WHERE user_id=:uid "
                   "AND table_id=:tid");
    query->bindValue(":tid", current_table_id);
    query->bindValue(":uid", uid);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    if (query->size() > 0)
    {
        emit queryError("User has read access");
        return;
    }
    
    query->prepare("INSERT INTO access_keys "
                   "VALUES (:tid, :uid, :key, :signedkey)");
    query->bindValue(":tid", current_table_id);
    query->bindValue(":uid", uid);
    query->bindValue(":key", Security::RSAEncrypt(security->getAESkey(), 
                                                  pubkey));
    query->bindValue(":signedkey", security->RSASign(security->getAESkey()));
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    
    emit rightsGranted();
}

//OK TESTED WORKING
void DBManager::grantWriteAccess(const QString &username, const QList<int> columns)
{
    query->prepare("SELECT user_id FROM users "
                   "WHERE user_name=:name");
    query->bindValue(":name", username);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    if (query->size() == 0)
    {
        emit queryError("User does not exist");
        return;
    }

    int uid = -1;
    while (query->next())
        uid = query->value(0).toInt();

    query->prepare("SELECT access_key "
                   "FROM access_keys "
                   "WHERE table_id=:table "
                   "AND user_id=:uid");
    query->bindValue(":table", current_table_id);
    query->bindValue(":uid", uid);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    if (query->size() == 0)
    {
        emit queryError("User does not have read access on this table");
        return;
    }

    for (int i=0; i<columns.length(); i++)
    {
        query->prepare("SELECT user_id "
                       "FROM rights "
                       "WHERE table_id=:tid "
                       "AND user_id=:uid "
                       "AND column_id=:cid");
        query->bindValue(":tid", current_table_id);
        query->bindValue(":uid", uid);
        query->bindValue(":cid", columns.at(i));
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return;
        }
        if (query->size() > 0)
            continue;

        query->prepare("INSERT INTO rights "
                       "VALUES (:tid, :uid, :cid)");
        query->bindValue(":tid", current_table_id);
        query->bindValue(":uid", uid);
        query->bindValue(":cid", columns.at(i));
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return;
        }
    }

    emit rightsLoaded(columns);
}

//OK, TESTED, WORKING
bool DBManager::writeData(int line, int column, const QString& cell_data)
{   
    QString dataToWrite;
    (cell_data == "")?(dataToWrite = ""):
            (dataToWrite = security->AESEncrypt(cell_data));
    query->prepare(QString("SELECT row_index FROM %1 "
                           "WHERE row_index = %2")
                           .arg(*current_table).arg(line));
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return false;
    }

    QString timestamp = QDate::currentDate().toString("dd/MM/yyyy");
    timestamp.append("&");
    timestamp.append(QTime::currentTime().toString("hh:mm:ss:zzz"));

    int size = query->size();
    
    if (size == 0)
    {
        if (dataToWrite == "")
            return false;
        
        QString q = QString("INSERT INTO %1 "
                            "(row_index, row_timestamp, row_height, field%2) "
                            "VALUES (%3, '%4', %5, '%6')").
                        arg(*current_table).arg(column).arg(line).
                        arg(timestamp).arg(spreadsheet->rowHeight(line)).
                        arg(dataToWrite);
        
        if (!query->exec(q))
        {
            emit queryError("Please check your database connection");
            return false;
        }
        spreadsheet->addTimestamp(line, timestamp);
        return true;
    }
    else if (size > 0)
    {
        QString q = QString("UPDATE %1 "
                            "SET row_timestamp = '%2', "
                            "field%3 = '%4' "
                            "WHERE row_index = %5").
                        arg(*current_table).
                        arg(timestamp).
                        arg(column).
                        arg(dataToWrite).
                        arg(line);
        if (!query->exec(q))
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

void DBManager::connectDB(const QString &uname, const QString &pass)
{
    db.setUserName(uname);
    db.setPassword(pass);
    if (!db.open())
    {
        QString error = db.lastError().text();
        emit queryError(QString("Unable to connect to database\nError: ").
                        append(error));
        /*
        emit initializeDatabaseRequest(uname, pass,
                                       QString("Unable to connect to database\nError: ").
                                       append(error));*/
    }
    else
    {
        query = new QSqlQuery(db);
        login(uname, pass);
    }
}

//OK, TESTED, WORKING
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

    int columns = query->record().count();
    if (columns == 0)
        return;

    spreadsheet->setColumnCount(columns-3);
    
    bool add = false;
    if (timestampCount == 0)
        add = true;
    QMap<int, QString> current_data = QMap<int, QString>();
    QMap<int,int> rows_height = QMap<int,int>();
    while (query->next())
    {
        aux.clear();
        for (int i=3; i<columns; i++)
        {
            QString data = query->value(i).toString();
            QString decryptedData;
            (data == "")?(decryptedData = ""):
                    (decryptedData = security->AESDecrypt(data));
            aux.append(decryptedData+"\n");
        }
        aux.remove(aux.length()-1, 1);
        current_data.insert(query->value(0).toInt(), aux);
        rows_height.insert(query->value(0).toInt(),
                           query->value(2).toInt());
        if (add)
            spreadsheet->addTimestamp(query->value(0).toInt(), query->value(1).toString());
    }

    QList<int> writable_columns = QList<int>();
    query->prepare("SELECT column_id "
                   "FROM rights "
                   "WHERE table_id=:tid "
                   "AND user_id=:uid");
    query->bindValue(":tid", current_table_id);
    query->bindValue(":uid", current_user_id);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    while (query->next())
        writable_columns.append(query->value(0).toInt());

    QMap<int,int> columns_width = QMap<int,int>();
    QMap<int,QString> headers_text = QMap<int,QString>();
    query->prepare("SELECT column_id, width, header_text "
                   "FROM tables_settings "
                   "WHERE table_id=:tid");
    query->bindValue(":tid", current_table_id);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    while (query->next())
    {
        columns_width.insert(query->value(0).toInt(),
                             query->value(1).toInt());
        headers_text.insert(query->value(0).toInt(),
                            query->value(2).toString());
    }
    emit columnsWidthLoaded(columns_width);
    emit rowsHeightLoaded(rows_height);
    emit columnsHeaderTextLoaded(headers_text);
    emit rightsLoaded(writable_columns);
    emit dataLoaded(current_data);
}

//OK, TESTED, WORKING
QHash<QString,QString> DBManager::getLinkData(const QHash<QString, QString> &matches) const
{
    QHash<QString,QString> result = QHash<QString,QString>();
    QString aux = "";
    QHashIterator<QString,QString> it(matches);
    while (it.hasNext())
    {
        it.next();
        QString link = QString("%1:%2").arg(it.key()).arg(it.value());
        QPair<int,int> id = SpreadSheet::getLocation(it.value());
        aux = QString("SELECT file_id, table_name "
                      "FROM files "
                      "WHERE file_name='%1'").arg(it.key());
        if (!query->exec(aux))
        {
            emit queryError("Please check your database connection");
            return QHash<QString,QString>();
        }
        int file_id = -1;
        QString table_name = "";
        while (query->next())
        {
            file_id = query->value(0).toInt();
            table_name = query->value(1).toString();
        }
        
        query->prepare("SELECT access_key "
                       "FROM access_keys "
                       "WHERE table_id=:tid "
                       "AND user_id=:uid");
        query->bindValue(":tid", file_id);
        query->bindValue(":uid", current_user_id);
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return QHash<QString,QString>();
        }
        if (query->size() == 0)
        {
            result.insertMulti(link, "#####");
            continue;
        }
        QString key = "";
        while (query->next())
            key = security->RSADecrypt(query->value(0).toString());
        
        aux = QString("SELECT field%1 FROM %2 "
                      "WHERE row_index=%3").
              arg(id.second).arg(table_name).arg(id.first);
        if (!query->exec(aux))
        {
            emit queryError("Please check your database connection");
            return QHash<QString,QString>();
        }
        aux = "";
        while (query->next())
            aux = security->AESDecrypt(query->value(0).toString(), key);
        result.insertMulti(link, aux);
    }
    return result;
}

//OK, TESTED, WORKING
void DBManager::getData(const QString &table)
{
    if (table.length() == 0)
        return;

    int rows = 0;
    QString aux = QString("SELECT file_id, table_name, row_count "
                          "FROM files "
                          "WHERE file_name='%1'").arg(table);
    if (!query->exec(aux))
    {
        emit queryError("Please check your database connection");
        return;
    }
    int file_id = -1;
    while (query->next())
    {
        file_id = query->value(0).toInt();
        aux = query->value(1).toString();
        rows = query->value(2).toInt();
    }
    
    query->prepare("SELECT access_key "
                   "FROM access_keys "
                   "WHERE table_id=:tid "
                   "AND user_id=:uid");
    query->bindValue(":tid", file_id);
    query->bindValue(":uid", current_user_id);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    if (query->size() == 0)
    {
        emit queryError("You don't have rights for reading this table");
        return;
    }
    QString key = "";
    while (query->next())
        key = security->RSADecrypt(query->value(0).toString());
    
    if (!query->exec(QString("SELECT * FROM %1").arg(aux)))
    {
        emit queryError("Please check your database connection");
        return;
    }
    
    int cols = query->record().count();
    emit setSpreadsheetSize(rows, cols-3); 
    QMap<int, QString> current_data = QMap<int, QString>();
    while (query->next())
    {
          aux.clear();
          for (int c=3; c<cols; c++)
          {
              QString data = query->value(c).toString();
              QString decryptedData;
              (data == "")?(decryptedData = ""):
                      (decryptedData = security->AESDecrypt(data, key));
              aux.append(decryptedData+"\n");
          }
          aux.remove(aux.length()-1, 1);
          current_data.insert(query->value(0).toInt(), aux);
    }
    emit givenDataLoaded(current_data);
    //emit rightsLoaded(QList<int>());
}

//OK, TESTED, WORKING
void DBManager::createTable(const QString &name, int columns,
                           int rows, const QString &folder)
{
    if (!query->exec("SELECT val "
                    "FROM current_ids "
                    "WHERE type='file'"))
    {
        emit queryError("Please check your database connection");
        return;
    }
    int current_index = 0;
    while (query->next())
        current_index = query->value(0).toInt();

    QString tableName = QString("table%1").arg(current_index);
    current_table = new QString(tableName);
    current_table_id = current_index;
    
    QString aux = QString("CREATE TABLE %1 (row_index INT NOT NULL, "
                          "row_timestamp VARCHAR(25) NOT NULL, "
                          "row_height INT NOT NULL, ").arg(tableName);
    for (int i=0; i<columns; i++)
        aux.append(QString("field%1 VARCHAR(2048), ").arg(i));
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
        emit queryError("Please check your database connection");
        return;
    }    

    query->prepare("INSERT INTO access_keys "
                   "VALUES (:tid, :uid, :key, :signature)");
    query->bindValue(":tid", current_table_id);
    query->bindValue(":uid", current_user_id);
    QString aesKey = Security::generateAESKey();
    if (aesKey == "")
    {
        deleteTable(current_table_id);
        emit queryError("Unable to generate private encryption key");
        return;
    }
    QString encrAccKey = security->RSAEncrypt(aesKey);
    if (encrAccKey == "")
    {
        deleteTable(current_table_id);
        emit queryError("Unable to encrypt the access key");
        return;
    }
    query->bindValue(":key", encrAccKey);
    QString accKeySigned = security->RSASign(aesKey);
    if (accKeySigned == "")
    {
        deleteTable(current_table_id);
        emit queryError("Unable to sign the access key");
        return;
    }
    query->bindValue(":signature", accKeySigned);
    security->setAESkey(aesKey);
    
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }

    for (int i=0; i<columns; i++)
        if (!query->exec(QString("INSERT INTO rights "
                                 "VALUES (%1, %2, %3)").arg(current_index).
                                            arg(current_user_id).arg(i)))
        {
            deleteTable(current_table_id);
            emit queryError("Please check your database connection");
            return;
        }
    
    query->prepare("UPDATE current_ids "
                   "SET val=val+1 "
                   "WHERE type='file'");
    if (!query->exec())
    {
        query->exec(QString("DROP TABLE %1").arg(tableName));
        query->prepare("DELETE FROM files WHERE file_id=:fileID");
        query->bindValue(":fileID", current_index);
        emit queryError("Please check your database connection");
        return;
    }

    emit tableCreated(tableName, columns, rows);
}

//OK, TESTED, WORKING
void DBManager::openTable(const QString &name, int columns,
              int rows, const QString &folder)
{   
    query->prepare("SELECT table_name, row_count, file_id, owner "
                   "FROM files "
                   "WHERE file_name = :name");
    query->bindValue(":name", name);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    int row_count = 0;
    int owner = -1;
    while (query->next())
    {
        current_table = new QString(query->value(0).toString());
        current_table_id = query->value(2).toInt();
        row_count = query->value(1).toInt();
        owner = query->value(3).toInt();
    }

    QString ownerPubKey = "";
    query->prepare("SELECT public_key "
                   "FROM users "
                   "WHERE user_id=:uid");
    query->bindValue(":uid", owner);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    while (query->next())
        ownerPubKey = query->value(0).toString();
    
    query->prepare("SELECT access_key, signed_key "
                   "FROM access_keys "
                   "WHERE table_id=:tid "
                   "AND user_id=:uid");
    query->bindValue(":tid", current_table_id);
    query->bindValue(":uid", current_user_id);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    if (query->size() == 0)
    {
        emit queryError("You don't have rights for reading this table");
        return;
    }
    while (query->next())
    {
        QString aesKey = security->RSADecrypt(query->value(0).toString());
        if (aesKey == "")
        {
            emit queryError("Unable to extract the file's key");
            return;
        }
        if (Security::RSAVerifySignature(aesKey, query->value(1).toString(),
                                         ownerPubKey))
        {
            if (!security->setAESkey(aesKey))
            {
                emit queryError("Unable to load the encryption key");
                return;
            }
        }
        else
        {
            emit queryError("Invalid access key - it's not given by the table's owner");
            return;
        }
    }

    if (!query->exec(QString("SELECT * FROM %1").arg(*current_table)))
        return;
    QSqlRecord record = query->record();
    int colCount = record.count() - 3;
    emit tableOpened(name, colCount, row_count);
}

//OK, TESTED, WORKING
int DBManager::columnCount()
{
    if (!query->exec(QString("SELECT * FROM %1").arg(*current_table)))
        return 0;
    QSqlRecord record = query->record();
    return record.count()-3;
}

int DBManager::loadUsers()
{
    query->prepare("SELECT user_name "
                   "FROM users "
                   "WHERE user_id <> :id");
    query->bindValue(":id", current_user_id);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return -1;
    }
    
    QList<QString> users = QList<QString>();
    while (query->next())
        users.append(query->value(0).toString());
    
    emit usersLoaded(users);
    return users.size();
}

//OK, TESTED, WORKING
QHash<QString, QString> DBManager::getTables()
{
    QHash<QString, QString> tables = QHash<QString, QString>();
    if (!query->exec("SELECT fi.file_name, fo.folder_name "
                     "FROM files fi, folders fo "
                     "WHERE fi.folder = fo.folder_id"))
        return tables;
    while (query->next())
        tables.insert(query->value(0).toString(), query->value(1).toString());
    return tables;
}

//OK, TESTED, WORKING
QHash<QString, QString> DBManager::getFolders()
{
    QHash<QString, QString> folders = QHash<QString, QString>();
    if (!query->exec("SELECT folder_name, folder_parent "
                     "FROM folders WHERE folder_id>0 "
                     "ORDER BY folder_id ASC"))
        return folders;
    while (query->next())
        folders.insert(query->value(0).toString(), query->value(1).toString());
    return folders;
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

void DBManager::disconnectDB()
{
    db.close();
}

void DBManager::deleteTable(int id)
{
    query->exec(QString("DROP TABLE table%1").arg(id));
    query->prepare("DELETE FROM files WHERE file_id=:fileID");
    query->bindValue(":fileID", id);
    query->exec();
}

//OK, TESTED, WORKING
void DBManager::login(const QString& uname, const QString& pass)
{
    if (!db.isOpen())
    {
        emit queryError("Not connected to the database\n"
                        "Please restart the application");
        return;
    }

    query->prepare("SELECT user_id, user_name, passwd, public_key "
                   "FROM users "
                   "WHERE user_name=:usr AND passwd=:pass");
    query->bindValue(":usr", uname);
    query->bindValue(":pass", Security::getHash(pass));
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    if (query->size() == 0)
    {
        createUser(uname, pass);
        return;
    }

    cfg->setCurrentUser(uname);
    QString passphrase = Security::getHash(uname+pass);
    while (query->next())
    {
        security->setRSAkeys(query->value(3).toString(),
                             cfg->getKey(), passphrase);
        current_user_id = query->value(0).toInt();
    }

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

//OK, TESTED, WORKING
void DBManager::createUser(const QString &uname, const QString &pass)
{
    query->prepare("SELECT user_id "
                   "FROM users "
                   "WHERE user_name=:usr");
    query->bindValue(":usr", uname);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
   
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

    QString passphr = Security::getHash(uname+pass);
    QPair<QString, QString> keys = Security::generateKeyPair(passphr);
    cfg->setCurrentUser(uname);
    cfg->setKey(keys.second);

    query->prepare("INSERT INTO users "
                   "VALUES (:uid, :usr, :pass, :pubkey)");
    query->bindValue(":uid", uid);
    query->bindValue(":usr", uname);
    query->bindValue(":pass", Security::getHash(pass));
    query->bindValue(":pubkey", keys.first);
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
    login(uname, pass);
}

//OK, TESTED, WORKING
void DBManager::addColumns(int columns)
{
    int fieldCount = columnCount();
    for (int i=0; i<columns; i++)
    {
        QString q = QString("ALTER TABLE %1 ADD COLUMN field%2 VARCHAR(2048)").
                    arg(*current_table).arg(fieldCount+i);
        if (!query->exec(q))
        {
            emit queryError("Please check your database connection");
            return;
        }

        q = QString("INSERT INTO rights VALUES (%1, %2, %3)").
                    arg(current_table_id).arg(current_user_id).arg(fieldCount+i);
        if (!query->exec(q))
        {
            emit queryError("Please check your database connection");
            return;
        }
    }
    emit columnsAdded(columns);
}

//OK, TESTED, WORKING
void DBManager::removeColumns(const QList <int> column_ids)
{
    QString q;
    for (int i=column_ids.length()-1; i>=0; i--)
    {
        q = QString("ALTER TABLE %1 DROP COLUMN field%2").
                    arg(*current_table).arg(column_ids.at(i));
        if (!query->exec(q))
        {
            emit queryError("Please check your database connection1");
            return;
        }
        //MODIFICARE PENTRU TOTI UTILIZATORII
        query->prepare("DELETE FROM rights "
                       "WHERE table_id=:tid "
                       "AND user_id=:uid "
                       "AND column_id=:cid");
        query->bindValue(":tid", current_table_id);
        query->bindValue(":uid", current_user_id);
        query->bindValue(":cid", column_ids.at(i));
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return;
        }
        
        query->prepare("UPDATE rights "
                       "SET column_id=column_id-1 "
                       "WHERE table_id=:tid "
                       "AND user_id=:uid "
                       "AND column_id>:cid");
        query->bindValue(":tid", current_table_id);
        query->bindValue(":uid", current_user_id);
        query->bindValue(":cid", column_ids.at(i));
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return;
        }
        
        query->prepare("DELETE FROM tables_settings "
                       "WHERE table_id=:tid "
                       "AND column_id=:cid");
        query->bindValue(":tid", current_table_id);
        query->bindValue(":cid", column_ids.at(i));
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return;
        }
        
        query->prepare("UPDATE tables_settings "
                       "SET column_id=column_id-1 "
                       "WHERE table_id=:tid "
                       "AND column_id>:cid");
        query->bindValue(":tid", current_table_id);
        query->bindValue(":cid", column_ids.at(i));
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
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
                "row_timestamp VARCHAR(25) NOT NULL, "
                "row_height INT NOT NULL, ").arg(*current_table);
    for (int i=0; i<newFieldCount; i++)
        q.append(QString("field%1 VARCHAR(2048), ").arg(i));
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

//OK, TESTED, WORKING
void DBManager::addRows(int rows)
{
    query->prepare("UPDATE files "
                   "SET row_count=row_count+:count "
                   "WHERE file_id=:fid");
    query->bindValue(":count", rows);
    query->bindValue(":fid", current_table_id);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    emit rowsAdded(rows);
}

//OK, TESTED, WORKING
void DBManager::setColumnWidth(int column, int oldSize, int newSize)
{
    query->prepare("SELECT width "
                   "FROM tables_settings "
                   "WHERE table_id=:tid "
                   "AND column_id=:col");
    query->bindValue(":tid", current_table_id);
    query->bindValue(":col", column);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    if (query->size() == 0)
    {
        query->prepare("INSERT INTO tables_settings "
                       "VALUES (:tid, :col, :width, :text)");
        query->bindValue(":tid", current_table_id);
        query->bindValue(":col", column);
        query->bindValue(":width", newSize);
        query->bindValue(":text", "");
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return;
        }
    }
    else
    {
        query->prepare("UPDATE tables_settings "
                       "SET width=:nw "
                       "WHERE table_id=:tid "
                       "AND column_id=:col "
                       "AND width <> :ow");
        query->bindValue(":nw", newSize);
        query->bindValue(":tid", current_table_id);
        query->bindValue(":col", column);
        query->bindValue(":ow", oldSize);
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return;
        }
    }
}

//OK, TESTED, WORKING
void DBManager::setColumnHeaderText(int column, const QString &text)
{
    query->prepare("SELECT header_text "
                   "FROM tables_settings "
                   "WHERE table_id=:tid "
                   "AND column_id=:col");
    query->bindValue(":tid", current_table_id);
    query->bindValue(":col", column);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    if (query->size() == 0)
    {
        query->prepare("INSERT INTO tables_settings "
                       "VALUES (:tid, :col, :width, :text)");
        query->bindValue(":tid", current_table_id);
        query->bindValue(":col", column);
        query->bindValue(":width", cfg->getCellsSize().width());
        query->bindValue(":text", text);
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return;
        }
    }
    else
    {
        query->prepare("UPDATE tables_settings "
                       "SET header_text=:txt "
                       "WHERE table_id=:tid "
                       "AND column_id=:col");
        query->bindValue(":txt", text);
        query->bindValue(":tid", current_table_id);
        query->bindValue(":col", column);
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return;
        }
    }
}

//OK, TESTED, WORKING
void DBManager::setRowHeight(int row, int oldSize, int newSize)
{
    query->prepare(QString("SELECT row_height FROM %1 "
                           "WHERE row_index=:ri").arg(*current_table));
    query->bindValue(":ri", row);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }

    QString timestamp = QDate::currentDate().toString("dd/MM/yyyy");
    timestamp.append("&");
    timestamp.append(QTime::currentTime().toString("hh:mm:ss:zzz"));

    if (query->size() == 0)
        query->prepare(QString("INSERT INTO %1 (row_index, row_timestamp, row_height) "
                               "VALUES (:row, :ts, :ns)").arg(*current_table));
    else
        query->prepare(QString("UPDATE %1 "
                               "SET row_height=:ns, row_timestamp=:ts "
                               "WHERE row_index=:row").arg(*current_table));
    query->bindValue(":ns", newSize);
    query->bindValue(":row", row);
    query->bindValue(":ts", timestamp);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
}

//OK, TESTED, WORKING
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

//TO BE TESTED
bool DBManager::removeFolder(const QString &name)
{
    bool removeChildren = cfg->removeChildren();

    query->prepare("SELECT folder_id FROM folders "
                   "WHERE folder_name=:fname");
    query->bindValue(":fname", name);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return false;
    }

    int folder_index = -1;
    while (query->next())
        folder_index = query->value(0).toInt();
    
    query->prepare("SELECT file_id "
                   "FROM files "
                   "WHERE folder=:fid "
                   "AND owner <> :uid");
    query->bindValue(":fid", folder_index);
    query->bindValue(":uid", current_user_id);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return false;
    }
    if (query->size() > 0)
    {
        emit queryError("Unable to delete - the folder contains tables you don't own");
        return false;
    }

    if (removeChildren)
    {
        query->prepare("SELECT folder_name "
                       "FROM folders "
                       "WHERE folder_parent=:fname");
        query->bindValue(":fname", name);
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return false;
        }
        QStringList subFolders = QStringList();
        while (query->next())
            subFolders.append(query->value(0).toString());
        QListIterator<QString> foldersIt(subFolders);
        while (foldersIt.hasNext())
        {
            QString folder = foldersIt.next();
            if (!removeFolder(folder))
            {
                emit queryError("Unable to remove subfolder "+folder);
                return false;
            }
        }
        
        query->prepare("SELECT file_name FROM files WHERE folder=:fid");
        query->bindValue(":fid", folder_index);
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return false;
        }
        QStringList files = QStringList();
        while (query->next())
            files.append(query->value(0).toString());
        QListIterator<QString> filesIt(files);
        while (filesIt.hasNext())
        {
            QString file = filesIt.next();
            if (!removeTable(file))
            {
                emit queryError("Unable to remove table "+file);
                return false;
            }
        }
    }
    else
    {
        query->prepare("UPDATE files SET folder=0 WHERE folder=:fid");
        query->bindValue(":fid", folder_index);
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return false;
        }

        query->prepare("UPDATE folders SET folder_parent='Root' "
                       "WHERE folder_parent=:fname");
        query->bindValue(":fname", name);
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return false;
        }
    }
    
    query->prepare("DELETE FROM folders "
                   "WHERE folder_id=:fid");
    query->bindValue(":fid", folder_index);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return false;
    }

    if (!query->exec("UPDATE current_ids "
                     "SET val=val-1 "
                     "WHERE type='folder'"))
    {
        emit queryError("Please check your database connection");
        return false;
    }
    
    emit dataModified(getTables(), getFolders());
    return true;
}

//OK, TESTED, WORKING
bool DBManager::removeTable(const QString& name)
{
    query->prepare("SELECT file_id, table_name, owner "
                   "FROM files "
                   "WHERE file_name=:fileName");
    query->bindValue(":fileName", name);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return false;
    }
    if (query->size() == 0)
        return false;

    QSqlRecord record = query->record();
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
    {
        emit queryError("Only the table's onwer can delete it");
        return false;
    }
    
    if (current_table->compare(tableName) == 0)
        emit closeCurrentTable();

    bool backupTables = cfg->backupTables();
    QString q = QString();
    if (backupTables)
    {       
        QDate expireDate;
        int daysToKeep = cfg->getBackupExpireDate();
        if (daysToKeep == 0)
            expireDate = QDate(2999, 12, 28);
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
            return false;
        }

        q = QString("INSERT INTO backup VALUES ('%1', '%2', %3)").
                arg(backupTableName).
                arg(expireDate.toString("yyyy-MM-dd")).
                arg(current_user_id);
        if (!query->exec(q))
        {
            emit queryError("Please check your database connection");
            return false;
        }
    }
    else
    {
        q = QString("DROP TABLE %1").arg(tableName);
        if (!query->exec(q))
        {
            emit queryError("Please check your database connection");
            return false;
        }
        
        query->prepare("DELETE FROM rights "
                       "WHERE table_id=:tid");
        query->bindValue(":tid", file_id);
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return false;
        }
        
        query->prepare("DELETE FROM tables_settings "
                       "WHERE table_id=:tid");
        query->bindValue(":tid", file_id);
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return false;
        }
        
        query->prepare("DELETE FROM access_keys "
                       "WHERE table_id=:tid");
        query->bindValue(":tid", file_id);
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            return false;
        }
        
        if (!query->exec("UPDATE current_ids "
                         "SET val=val-1 "
                         "WHERE type='file'"))
        {
            emit queryError("Please check your database connection");
            return false;
        }
    }

    query->prepare("DELETE FROM files WHERE file_id=:fid");
    query->bindValue(":fid", file_id);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return false;
    }
    
    emit dataModified(getTables(), getFolders());
    return true;
}

//OK, TESTED, WORKING
void DBManager::changeKey(const QString &oldPrivateKey, 
                          const QString &publicKey, 
                          const QString &privateKey, 
                          const QString &passphrase)
{
    if (!security->setRSAkeys(publicKey, privateKey, passphrase))
    {
        emit queryError("Invalid keys");
        return;
    }
    cfg->setKey(privateKey);

    query->prepare("UPDATE users "
                   "SET public_key=:key "
                   "WHERE user_id=:uid");
    query->bindValue(":key", publicKey);
    query->bindValue(":uid", current_user_id);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }

    query->prepare("SELECT table_id, access_key "
                   "FROM access_keys "
                   "WHERE user_id=:uid");
    query->bindValue(":uid", current_user_id);
    if (!query->exec())
    {
        emit queryError("Please check your database connection");
        return;
    }
    
    QHash<int,QString> accessKeys = QHash<int,QString>();
    while (query->next())
        accessKeys.insert(query->value(0).toInt(),
                          query->value(1).toString());
    QHashIterator<int,QString> it(accessKeys);
    while (it.hasNext())
    {
        it.next();
        QString accKey = Security::RSADecrypt(it.value(),
                                              oldPrivateKey,
                                              passphrase);
        query->prepare("UPDATE access_keys "
                       "SET access_key=:newKey, "
                       "signed_key=:newSign "
                       "WHERE user_id=:uid "
                       "AND table_id=:tid");
        query->bindValue(":newKey", security->RSAEncrypt(accKey));
        query->bindValue(":newSign", security->RSASign(accKey));
        query->bindValue(":uid", current_user_id);
        query->bindValue(":tid", it.key());
        if (!query->exec())
        {
            emit queryError("Please check your database connection");
            continue;
        }
    }
}

#include "DBManager.h"

DBManager::DBManager(const CFGManager *cfg)
{
    db = QSqlDatabase::addDatabase(cfg->getDBType());
    db.setHostName(cfg->getDBServer());
    db.setDatabaseName(cfg->getDBName());

    current_table = new QString();
    current_user_id = -1;

    security = new Security(cfg->getKey());
}

void DBManager::setCurrentSpreadSheet(SpreadSheet *spreadsheet)
{
    this->spreadsheet = spreadsheet;
}

void DBManager::removeCurrentData()
{
    current_table->clear();
}

bool DBManager::connectDB(const QString &uname, const QString &pass)
{
    db.setUserName(uname);
    db.setPassword(pass);
    db.open();
    if (!db.isOpen())
        return false;
    else
    {
        query = new QSqlQuery(db);
        return true;
    }
}

void DBManager::login(const QString& uname, const QString& pass)
{
    QString username = security->encryptData(uname);
    QString password = security->encryptData(pass);
    query->prepare("SELECT user_id, user_name, passwd "
                   "FROM users "
                   "WHERE user_name=:usr AND passwd=:pass");
    query->bindValue(":usr",username);
    query->bindValue(":pass",password);
    if (!query->exec())
        return;
    if (query->size() <= 0)
    {
        emit loggedIn(-1);
        return;
    }

    QSqlRecord record = query->record();
    while (query->next())
        current_user_id = query->value(record.indexOf("user_id")).toInt();
    emit loggedIn(current_user_id);
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
        return false;
    int size = query->size();
    if (size == 0)
    {
        QString timestamp = QDate::currentDate().toString("dd/MM/yyyy");
        timestamp.append("&");
        timestamp.append(QTime::currentTime().toString("hh:mm:ss:zzz"));

        query->prepare(QString("INSERT INTO %1"
                       "(row_index, %2, owner, row_timestamp)"
                       " VALUES (:rowIndex, :fieldVal, "
                                ":owner, :timestamp)").
                       arg(*current_table).arg(QString("field%1").
                       arg(column)));
        query->bindValue(":rowIndex", line);
        query->bindValue(":fieldVal", dataToWrite);
        query->bindValue(":owner", current_user_id);
        query->bindValue(":timestamp", timestamp);
        if (!query->exec())
            return false;

        spreadsheet->addTimestamp(timestamp);
        return true;
    }
    else if (size > 0)
    {
        QString timestamp = QDate::currentDate().toString("dd/MM/yyyy");
        timestamp.append("&");
        timestamp.append(QTime::currentTime().toString("hh:mm:ss:zzz"));

        query->prepare(QString("UPDATE %1 "
                               "SET %2 = :fieldVal, "
                               "row_timestamp = :timestamp "
                               "WHERE row_index = :rowIndex").
                                arg(*current_table).
                                arg(QString("field%1").arg(column)));
        query->bindValue(":fieldVal", dataToWrite);
        query->bindValue(":timestamp", timestamp);
        query->bindValue(":rowIndex", line);
        if (!query->exec())
            return false;

        spreadsheet->replaceTimestamp(line, timestamp);
        return true;
    }
    return false;
}

bool DBManager::deleteFile(const QString& name, int client_id)
{
    query->prepare("SELECT file_id, table_name, owner "
                   "FROM files "
                   "WHERE file_name=:fileName");
    query->bindValue(":fileName", name);
    if (query->exec())
    {
        QSqlRecord record = query->record();
        if (record.count() > 0)
        {
            int file_id = 0;
            int owner = 0;
            while (query->next())
            {
                file_id = query->value(record.indexOf("file_id")).toInt();
                owner = query->value(record.indexOf("owner")).toInt();
            }
            if (owner != client_id)
                return false;
            query->prepare("DROP TABLE :tableName");
            query->bindValue(":tableName", name);
            if (query->exec())
            {
                query->prepare("DELETE FROM :tableName WHERE file_id=:f_id");
                query->bindValue(":f_id", file_id);
                return query->exec();
            }
        }
    }
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

    //qDebug() << aux;
    if (!query->exec(aux))
    {
        qDebug() << "query error";
        return;
    }

    QSqlRecord record = query->record();
    int columns = record.count();
    if (columns == 0)
        return;

    columns -= 2;
    bool add = false;
    if (timestampCount == 0)
        add = true;
    QStringList current_data = QStringList();
    while (query->next())
    {
        aux.clear();
        for (int i=1; i<columns; i++)
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

void DBManager::createFile(const QString &name, int columns,
                           int rows, const QString &folder)
{
    QSqlRecord record;
    if (!query->exec("SELECT * "
                    "FROM current_ids "
                    "WHERE type='file'"))
        return;
    record = query->record();
    int colIndex = record.indexOf("val");
    int current_index = 0;
    while (query->next())
        current_index = query->value(colIndex).toInt();

    QString tableName = QString("table%1").arg(current_index);
    QString aux = QString("CREATE TABLE %1 (row_index INT NOT NULL, ").
                            arg(tableName);
    for (int i=0; i<columns; i++)
        aux.append(QString("field%1 VARCHAR(512), ").arg(i));
    aux.append("owner INT NOT NULL, "
               "row_timestamp VARCHAR(25) NOT NULL, "
               "CONSTRAINT row_index_pk "
               "PRIMARY KEY (row_index), "
               "CONSTRAINT owner_fk "
               "FOREIGN KEY (owner) "
               "REFERENCES users (user_id) )");
    query->prepare(aux);
    if (!query->exec())
        return;

    query->prepare("INSERT INTO files "
                   "VALUES (:id, :tableName, :fileName, :owner, "
                   "(SELECT folder_id FROM folders WHERE folder_name = :folder) )");
    query->bindValue(":id", current_index);
    query->bindValue(":tableName", tableName);
    query->bindValue(":fileName", name);
    query->bindValue(":owner", current_user_id);
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

void DBManager::openFile(const QString &name, int columns,
              int rows, const QString &folder)
{
    query->prepare("SELECT table_name FROM files "
                   "WHERE file_name = :name");
    query->bindValue(":name", name);
    if (!query->exec())
        return;
    while (query->next())
        current_table = new QString(query->value(0).toString());

    query->exec(QString("SELECT * FROM %1").arg(*current_table));
    QSqlRecord record = query->record();
    int colCount = record.count() - 3;
    emit fileOpened(name, colCount, rows);
}

int DBManager::columnCount()
{
    if (!query->exec(QString("SELECT * FROM %1").arg(*current_table)))
        return 0;
    QSqlRecord record = query->record();
    return record.count()-3;
}

QMap<QString, QString> DBManager::getFiles()
{
    QMap<QString, QString> files = QMap<QString, QString>();
    if (!query->exec("SELECT fi.file_name, fo.folder_name "
                     "FROM files fi, folders fo "
                     "WHERE fi.folder = fo.folder_id"))
        return files;
    while (query->next())
        files.insert(query->value(0).toString(),
                     query->value(1).toString());
    return files;
}

QMap<QString, QString> DBManager::getDirectories()
{
    QMap<QString, QString> dirs = QMap<QString, QString>();
    if (!query->exec("SELECT folder_name, folder_parent "
                     "FROM folders"))
        return dirs;
    while (query->next())
        dirs.insert(query->value(0).toString(),
                     query->value(1).toString());
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

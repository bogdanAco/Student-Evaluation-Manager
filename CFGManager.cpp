#include "CFGManager.h"

CFGManager::CFGManager()
{
    domDoc = new QDomDocument();
    XMLFile = new QFile("config.xml");
    if (!XMLFile->exists())
    {
        root = new QDomElement(domDoc->createElement("configuration"));
        domDoc->appendChild(root->toElement());

        QDomElement database = domDoc->createElement("database");
        root->appendChild(database);
        QDomElement dbType = domDoc->createElement("type");
        dbType.appendChild(domDoc->createTextNode("QMYSQL"));
        database.appendChild(dbType);
        QDomElement dbServer = domDoc->createElement("server");
        dbServer.appendChild(domDoc->createTextNode("localhost"));
        database.appendChild(dbServer);
        QDomElement dbPort = domDoc->createElement("port");
        dbPort.appendChild(domDoc->createTextNode("-1"));
        database.appendChild(dbPort);
        QDomElement dbName = domDoc->createElement("name");
        dbName.appendChild(domDoc->createTextNode("studMng"));
        database.appendChild(dbName);
        QDomElement dbUser = domDoc->createElement("user");
        dbUser.appendChild(domDoc->createTextNode("root"));
        database.appendChild(dbUser);
        QDomElement dbPass = domDoc->createElement("password");
        dbPass.appendChild(domDoc->createTextNode(" "));
        database.appendChild(dbPass);
        QDomElement removeChildren =
                domDoc->createElement("delete_folder_content");
        removeChildren.appendChild(domDoc->createTextNode("true"));
        database.appendChild(removeChildren);
        QDomElement backupTables =
                domDoc->createElement("backup_deleted_tables");
        backupTables.appendChild(domDoc->createTextNode("true"));
        database.appendChild(backupTables);
        QDomElement expireDate = domDoc->createElement("backup_expire_after");
        expireDate.appendChild(domDoc->createTextNode("0"));
        database.appendChild(expireDate);

        QDomElement spreadsheet = domDoc->createElement("spreadsheet");
        root->appendChild(spreadsheet);
        QDomElement rows = domDoc->createElement("rows");
        rows.appendChild(domDoc->createTextNode("600"));
        spreadsheet.appendChild(rows);
        QDomElement columns = domDoc->createElement("columns");
        columns.appendChild(domDoc->createTextNode("6"));
        spreadsheet.appendChild(columns);
        QDomElement refresh = domDoc->createElement("refresh_time");
        refresh.appendChild(domDoc->createTextNode("5"));
        spreadsheet.appendChild(refresh);
        QDomElement height = domDoc->createElement("row_height");
        height.appendChild(domDoc->createTextNode("25"));
        spreadsheet.appendChild(height);
        QDomElement width = domDoc->createElement("column_width");
        width.appendChild(domDoc->createTextNode("100"));
        spreadsheet.appendChild(width);
        QDomElement selection = domDoc->createElement("selection_mode");
        selection.appendChild(domDoc->createTextNode("ExtendedSelection"));
        spreadsheet.appendChild(selection);

        QDomElement security = domDoc->createElement("security");
        root->appendChild(security);
        QDomElement key = domDoc->createElement("key");
        key.appendChild(domDoc->createTextNode("621ffdd4e90bf0122a99976b7ccfb031"));
        security.appendChild(key);
        QDomElement algorithm = domDoc->createElement("algorithm");
        algorithm.appendChild(domDoc->createTextNode("aes128"));
        security.appendChild(algorithm);

        saveDoc();
    }
    else
        if (XMLFile->open(QFile::ReadOnly | QFile::Text))
        {
            if (domDoc->setContent(XMLFile))
                root = new QDomElement(domDoc->documentElement());
            XMLFile->close();
        }

    QFile f("personal.key");
    if (!f.open(QIODevice::ReadOnly | QFile::Text))
    {
        pKey = new QString();
        return;
    }
    QByteArray key = f.readAll();
    f.close();
    pKey = new QString(key);
}

CFGManager::~CFGManager()
{
    delete pKey;
    delete domDoc;
    delete root;
    delete XMLFile;
}

QString CFGManager::getDBType() const
{
    return root->firstChildElement("database").
            firstChildElement("type").text();
}

QString CFGManager::getDBServer() const
{
    return root->firstChildElement("database").
            firstChildElement("server").text();
}

int CFGManager::getDBPort() const
{
    return root->firstChildElement("database").
            firstChildElement("port").text().toInt();
}

QString CFGManager::getDBName() const
{
    return root->firstChildElement("database").
            firstChildElement("name").text();
}

QString CFGManager::getDBUser() const
{
    return root->firstChildElement("database").
            firstChildElement("user").text();
}

QString CFGManager::getDBPassword() const
{
    QString aux = root->firstChildElement("database").
                  firstChildElement("password").text();
    return (aux != " ")?aux:"";
}

bool CFGManager::removeChildren() const
{
    QString aux = root->firstChildElement("database").
                  firstChildElement("delete_folder_content").
                  text();
    return (aux == "true")?true:false;
}

bool CFGManager::backupTables() const
{
    QString aux = root->firstChildElement("database").
                  firstChildElement("backup_deleted_tables").
                  text();
    return (aux == "true")?true:false;
}

int CFGManager::getBackupExpireDate() const
{
    return root->firstChildElement("database").
            firstChildElement("backup_expire_after").text().toInt();
}

int CFGManager::getRowCount() const
{
    return root->firstChildElement("spreadsheet").
            firstChildElement("rows").text().toInt();
}

int CFGManager::getColumnCount() const
{
    return root->firstChildElement("spreadsheet").
            firstChildElement("columns").text().toInt();
}

int CFGManager::getRefreshTime() const
{
    return root->firstChildElement("spreadsheet").
            firstChildElement("refresh_time").text().toInt();
}

int CFGManager::getSelectionMode() const
{
    QString mode =  root->firstChildElement("spreadsheet").
                    firstChildElement("selection_mode").text();
    if (mode == "SingleSelection")
        return 1;
    else if (mode == "ContiguousSelection")
        return 4;
    else if (mode == "ExtendedSelection")
        return 3;
    else if (mode == "MultiSelection")
        return 2;
    else
        return 0; //NoSelection
}

QString CFGManager::getSelectionModeText() const
{
    return root->firstChildElement("spreadsheet").
            firstChildElement("selection_mode").text();
}

QSize CFGManager::getCellsSize() const
{
    QSize aux = QSize();
    aux.setHeight(root->firstChildElement("spreadsheet").
                  firstChildElement("row_height").text().toInt());
    aux.setWidth(root->firstChildElement("spreadsheet").
                 firstChildElement("column_width").text().toInt());
    return aux;
}

QString CFGManager::getKey() const
{
    return root->firstChildElement("security").
            firstChildElement("key").text();
}

QString CFGManager::getAlgorithm() const
{
    return root->firstChildElement("security").
            firstChildElement("algorithm").text();
}

bool CFGManager::pKeyExists() const
{
    QFile f("personal.key");
    return f.exists();
}

QString CFGManager::getPKey() const
{
    if (pKey->length() != 0)
        return QString(*pKey);

    QFile f("personal.key");
    if (!f.open(QIODevice::ReadOnly | QFile::Text))
        return "";
    QByteArray key = f.readAll();
    f.close();
    pKey->clear();
    pKey->append(key);
    return QString(key);
}

void CFGManager::saveDoc() const
{
    if (XMLFile->open(QFile::WriteOnly | QFile::Text))
    {
        QTextStream out(XMLFile);
        domDoc->save(out, 4);
        XMLFile->flush();
        XMLFile->close();
    }

    QFile f("personal.key");
    if (!f.open(QIODevice::WriteOnly | QFile::Text))
        return;
    QByteArray aux = pKey->toAscii();
    f.write(aux);
    f.flush();
    f.close();
}

void CFGManager::undoDoc() const
{
    if (XMLFile->open(QFile::ReadOnly | QFile::Text))
    {
        if (domDoc->setContent(XMLFile))
        {
            delete root;
            root = new QDomElement(domDoc->documentElement());
        }
        XMLFile->close();
    }
}

void CFGManager::setDBType(const QString &type)
{
    QString databaseType;
    if (type == "MySQL")
        databaseType = "QMYSQL";
    else if (type == "PostgreSQL")
        databaseType = "QPSQL";
    else if (type == "Oracle")
        databaseType = "QOCI";
    else
        databaseType = "";
    QDomElement currentValue(root->firstChildElement("database").
                            firstChildElement("type"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(databaseType));
}

void CFGManager::setDBServer(const QString &server)
{
    QDomElement currentValue(root->firstChildElement("database").
                            firstChildElement("server"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(server));
}

void CFGManager::setDBPort(int port)
{
    QDomElement currentValue(root->firstChildElement("database").
                            firstChildElement("port"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(QString("%1").arg(port)));
}

void CFGManager::setDBName(const QString &name)
{
    QDomElement currentValue(root->firstChildElement("database").
                            firstChildElement("name"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(name));
}

void CFGManager::setDBUser(const QString &user)
{
    QDomElement currentValue(root->firstChildElement("database").
                            firstChildElement("user"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(user));
}

void CFGManager::setDBPassword(const QString &pass)
{
    QString aux = (pass == "")?" ":pass;
    QDomElement currentValue(root->firstChildElement("database").
                            firstChildElement("password"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(aux));
}

void CFGManager::setRemoveChildren(bool remove)
{
    QString aux = remove?"true":"false";
    QDomElement currentValue(root->firstChildElement("database").
                            firstChildElement("delete_folder_content"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(aux));
}

void CFGManager::setBackupTables(bool backup)
{
    QString aux = backup?"true":"false";
    QDomElement currentValue(root->firstChildElement("database").
                            firstChildElement("backup_deleted_tables"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(aux));
}

void CFGManager::setBackupExpireDate(int afterNDays)
{
    QDomElement currentValue(root->firstChildElement("database").
                            firstChildElement("backup_expire_after"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(QString("%1").arg(afterNDays)));
}

void CFGManager::setRowCount(int count)
{
    QDomElement currentValue(root->firstChildElement("spreadsheet").
                            firstChildElement("rows"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(QString("%1").arg(count)));
}

void CFGManager::setColumnCount(int count)
{
    QDomElement currentValue(root->firstChildElement("spreadsheet").
                            firstChildElement("columns"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(QString("%1").arg(count)));
}

void CFGManager::setRefreshTime(int seconds)
{
    QDomElement currentValue(root->firstChildElement("spreadsheet").
                            firstChildElement("refresh_time"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(QString("%1").arg(seconds)));
}

void CFGManager::setSelectionMode(const QString &mode)
{
    QDomElement currentValue(root->firstChildElement("spreadsheet").
                            firstChildElement("selection_mode"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(mode));
}

void CFGManager::setCellsSize(const QSize &size)
{
    QDomElement currentHeight(root->firstChildElement("spreadsheet").
                            firstChildElement("row_height"));
    currentHeight.removeChild(currentHeight.firstChild());
    currentHeight.appendChild(domDoc->createTextNode(QString("%1").arg(size.height())));

    QDomElement currentWidth(root->firstChildElement("spreadsheet").
                            firstChildElement("column_width"));
    currentWidth.removeChild(currentWidth.firstChild());
    currentWidth.appendChild(domDoc->createTextNode(QString("%1").arg(size.width())));
}

void CFGManager::setRowHeight(int height)
{
    QDomElement currentHeight(root->firstChildElement("spreadsheet").
                            firstChildElement("row_height"));
    currentHeight.removeChild(currentHeight.firstChild());
    currentHeight.appendChild(domDoc->createTextNode(QString("%1").arg(height)));
}

void CFGManager::setColumnWidth(int width)
{
    QDomElement currentWidth(root->firstChildElement("spreadsheet").
                            firstChildElement("column_width"));
    currentWidth.removeChild(currentWidth.firstChild());
    currentWidth.appendChild(domDoc->createTextNode(QString("%1").arg(width)));
}

void CFGManager::setKey(const QString &key)
{
    if (key.length() != 32)
    {
        emit errorMessage("Invalid key size (must be 32)");
        return;
    }

    QDomElement currentValue(root->firstChildElement("security").
                            firstChildElement("key"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(key));
}

void CFGManager::setPKey(const QString &key) const
{
    if (key.length() != 32)
    {
        emit errorMessage("Invalid key size (must be 32)");
        return;
    }
    pKey->clear();
    pKey->append(key);
}

void CFGManager::setAlgorithm(const QString &alg)
{
    QDomElement currentValue(root->firstChildElement("security").
                            firstChildElement("algorithm"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(alg));
}

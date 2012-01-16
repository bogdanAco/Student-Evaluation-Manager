#include "CFGManager.h"

CFGManager::CFGManager()
{
    root = 0;
    currentUser = 0;
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
        saveDoc();
    }
    else
    {
        if (XMLFile->open(QFile::ReadOnly | QFile::Text))
        {
            if (domDoc->setContent(XMLFile))
                root = new QDomElement(domDoc->documentElement());
            XMLFile->close();
        }
    }
}

CFGManager::~CFGManager()
{
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

bool CFGManager::removeChildren() const
{
    if (currentUser == 0)
    {
        emitErrorMessage(NoUser);
        return false;
    }

    QString aux = currentUser->firstChildElement("tables").
                  firstChildElement("delete_folder_content").
                  text();
    return (aux == "true")?true:false;
}

bool CFGManager::backupTables() const
{
    if (currentUser == 0)
    {
        emitErrorMessage(NoUser);
        return "";
    }

    QString aux = currentUser->firstChildElement("tables").
                  firstChildElement("backup_deleted_tables").
                  text();
    return (aux == "true")?true:false;
}

int CFGManager::getBackupExpireDate() const
{
    if (currentUser == 0)
    {
        emitErrorMessage(NoUser);
        return -1;
    }

    return currentUser->firstChildElement("tables").
            firstChildElement("backup_expire_after").text().toInt();
}

int CFGManager::getRowCount() const
{
    if (currentUser == 0)
    {
        emitErrorMessage(NoUser);
        return -1;
    }

    return currentUser->firstChildElement("spreadsheet").
            firstChildElement("rows").text().toInt();
}

int CFGManager::getColumnCount() const
{
    if (currentUser == 0)
    {
        emitErrorMessage(NoUser);
        return -1;
    }

    return currentUser->firstChildElement("spreadsheet").
            firstChildElement("columns").text().toInt();
}

int CFGManager::getRefreshTime() const
{
    if (currentUser == 0)
    {
        emitErrorMessage(NoUser);
        return -1;
    }

    return currentUser->firstChildElement("spreadsheet").
            firstChildElement("refresh_time").text().toInt();
}

QAbstractItemView::SelectionMode CFGManager::getSelectionMode() const
{
    if (currentUser == 0)
    {
        emitErrorMessage(NoUser);
        return QAbstractItemView::NoSelection;
    }

    QString mode =  currentUser->firstChildElement("spreadsheet").
                    firstChildElement("selection_mode").text();
    if (mode == "SingleSelection")
        return QAbstractItemView::SingleSelection;
    else if (mode == "ContiguousSelection")
        return QAbstractItemView::ContiguousSelection;
    else if (mode == "ExtendedSelection")
        return QAbstractItemView::ExtendedSelection;
    else if (mode == "MultiSelection")
        return QAbstractItemView::MultiSelection;
    else
        return QAbstractItemView::NoSelection;
}

QString CFGManager::getSelectionModeText() const
{
    if (currentUser == 0)
    {
        emitErrorMessage(NoUser);
        return "";
    }

    return currentUser->firstChildElement("spreadsheet").
            firstChildElement("selection_mode").text();
}

QSize CFGManager::getCellsSize() const
{
    if (currentUser == 0)
    {
        emitErrorMessage(NoUser);
        return QSize();
    }   

    QSize aux = QSize();
    aux.setHeight(currentUser->firstChildElement("spreadsheet").
                  firstChildElement("row_height").text().toInt());
    aux.setWidth(currentUser->firstChildElement("spreadsheet").
                 firstChildElement("column_width").text().toInt());
    return aux;
}

QString CFGManager::getKey() const
{
    if (currentUser == 0)
    {
        emitErrorMessage(NoUser);
        return "";
    }

    return currentUser->firstChildElement("security").
            firstChildElement("key").text();
}

void CFGManager::emitErrorMessage(ErrorMessage m) const
{
    if (m == NoUser)
        emit errorMessage("No current user is set");
}

void CFGManager::setCurrentUser(const QString &name) const
{
    delete currentUser;
    QDomElement node = root->firstChildElement(name);
    if (node.isNull())
    {
        currentUser = new QDomElement(domDoc->createElement(name));
            QDomElement database = domDoc->createElement("tables");
            currentUser->appendChild(database);
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
            currentUser->appendChild(spreadsheet);
                QDomElement rows = domDoc->createElement("rows");
                rows.appendChild(domDoc->createTextNode("100"));
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
        currentUser->appendChild(security);
            QDomElement key = domDoc->createElement("key");
            key.appendChild(domDoc->createTextNode(" "));
            security.appendChild(key);

        root->appendChild(*currentUser);
    }
    else
        currentUser = new QDomElement(node);
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

void CFGManager::setRemoveChildren(bool remove)
{
    if (currentUser == 0)
    {
        emitErrorMessage(NoUser);
        return;
    }

    QString aux = remove?"true":"false";
    QDomElement currentValue(currentUser->firstChildElement("tables").
                            firstChildElement("delete_folder_content"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(aux));
}

void CFGManager::setBackupTables(bool backup)
{
    if (currentUser == 0)
    {
        emitErrorMessage(NoUser);
        return;
    }

    QString aux = backup?"true":"false";
    QDomElement currentValue(currentUser->firstChildElement("tables").
                            firstChildElement("backup_deleted_tables"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(aux));
}

void CFGManager::setBackupExpireDate(int afterNDays)
{
    if (currentUser == 0)
    {
        emitErrorMessage(NoUser);
        return;
    }

    QDomElement currentValue(currentUser->firstChildElement("tables").
                            firstChildElement("backup_expire_after"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(QString("%1").arg(afterNDays)));
}

void CFGManager::setRowCount(int count)
{
    if (currentUser == 0)
    {
        emitErrorMessage(NoUser);
        return;
    }

    QDomElement currentValue(currentUser->firstChildElement("spreadsheet").
                            firstChildElement("rows"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(QString("%1").arg(count)));
}

void CFGManager::setColumnCount(int count)
{
    if (currentUser == 0)
    {
        emitErrorMessage(NoUser);
        return;
    }

    QDomElement currentValue(currentUser->firstChildElement("spreadsheet").
                            firstChildElement("columns"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(QString("%1").arg(count)));
}

void CFGManager::setRefreshTime(int seconds)
{
    if (currentUser == 0)
    {
        emitErrorMessage(NoUser);
        return;
    }

    QDomElement currentValue(currentUser->firstChildElement("spreadsheet").
                            firstChildElement("refresh_time"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(QString("%1").arg(seconds)));
}

void CFGManager::setSelectionMode(const QString &mode)
{
    if (currentUser == 0)
    {
        emitErrorMessage(NoUser);
        return;
    }

    QDomElement currentValue(currentUser->firstChildElement("spreadsheet").
                            firstChildElement("selection_mode"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(mode));
}

void CFGManager::setCellsSize(const QSize &size)
{
    {
        emitErrorMessage(NoUser);
        return;
    }

    QDomElement currentHeight(currentUser->firstChildElement("spreadsheet").
                            firstChildElement("row_height"));
    currentHeight.removeChild(currentHeight.firstChild());
    currentHeight.appendChild(domDoc->createTextNode(QString("%1").arg(size.height())));

    QDomElement currentWidth(currentUser->firstChildElement("spreadsheet").
                            firstChildElement("column_width"));
    currentWidth.removeChild(currentWidth.firstChild());
    currentWidth.appendChild(domDoc->createTextNode(QString("%1").arg(size.width())));
}

void CFGManager::setRowHeight(int height)
{
    if (currentUser == 0)
    {
        emitErrorMessage(NoUser);
        return;
    }

    QDomElement currentHeight(currentUser->firstChildElement("spreadsheet").
                            firstChildElement("row_height"));
    currentHeight.removeChild(currentHeight.firstChild());
    currentHeight.appendChild(domDoc->createTextNode(QString("%1").arg(height)));
}

void CFGManager::setColumnWidth(int width)
{
    if (currentUser == 0)
    {
        emitErrorMessage(NoUser);
        return;
    }

    QDomElement currentWidth(currentUser->firstChildElement("spreadsheet").
                            firstChildElement("column_width"));
    currentWidth.removeChild(currentWidth.firstChild());
    currentWidth.appendChild(domDoc->createTextNode(QString("%1").arg(width)));
}

void CFGManager::setKey(const QString &key) const
{
    if (currentUser == 0)
    {
        emitErrorMessage(NoUser);
        return;
    }

    QDomElement currentValue(currentUser->firstChildElement("security").
                            firstChildElement("key"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(key));

    saveDoc();
}

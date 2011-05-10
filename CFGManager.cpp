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
        QDomElement dbName = domDoc->createElement("name");
        dbName.appendChild(domDoc->createTextNode("studMng"));
        database.appendChild(dbName);
        QDomElement dbUser = domDoc->createElement("user");
        dbUser.appendChild(domDoc->createTextNode("root"));
        database.appendChild(dbUser);
        QDomElement dbPass = domDoc->createElement("password");
        dbPass.appendChild(domDoc->createTextNode(" "));
        database.appendChild(dbPass);

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
}

CFGManager::~CFGManager()
{
    delete domDoc;
    delete root;
    delete XMLFile;
}

void CFGManager::saveDoc()
{
    if (XMLFile->open(QFile::WriteOnly | QFile::Text))
    {
        QTextStream out(XMLFile);
        domDoc->save(out, 4);
        XMLFile->flush();
        XMLFile->close();
    }
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

void CFGManager::setDBType(const QString &type)
{
    QDomElement currentValue(root->firstChildElement("database").
                            firstChildElement("type"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(type));
    saveDoc();
}

void CFGManager::setDBServer(const QString &server)
{
    QDomElement currentValue(root->firstChildElement("database").
                            firstChildElement("server"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(server));
    saveDoc();
}

void CFGManager::setDBName(const QString &name)
{
    QDomElement currentValue(root->firstChildElement("database").
                            firstChildElement("name"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(name));
    saveDoc();
}

void CFGManager::setDBUser(const QString &user)
{
    QDomElement currentValue(root->firstChildElement("database").
                            firstChildElement("user"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(user));
    saveDoc();
}

void CFGManager::setDBPassword(const QString &pass)
{
    QString aux = (pass == "")?" ":pass;
    QDomElement currentValue(root->firstChildElement("database").
                            firstChildElement("password"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(aux));
    saveDoc();
}

void CFGManager::setRowCount(int count)
{
    QDomElement currentValue(root->firstChildElement("spreadsheet").
                            firstChildElement("rows"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(QString("%1").arg(count)));
    saveDoc();
}

void CFGManager::setColumnCount(int count)
{
    QDomElement currentValue(root->firstChildElement("spreadsheet").
                            firstChildElement("columns"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(QString("%1").arg(count)));
    saveDoc();
}

void CFGManager::setRefreshTime(int seconds)
{
    QDomElement currentValue(root->firstChildElement("spreadsheet").
                            firstChildElement("refresh_time"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(QString("%1").arg(seconds)));
    saveDoc();
}

void CFGManager::setSelectionMode(const QString &mode)
{
    QDomElement currentValue(root->firstChildElement("spreadsheet").
                            firstChildElement("selection_mode"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(mode));
    saveDoc();
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
    saveDoc();
}

void CFGManager::setKey(const QString &key)
{
    QDomElement currentValue(root->firstChildElement("security").
                            firstChildElement("key"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(key));
    saveDoc();
}

void CFGManager::setAlgorithm(const QString &alg)
{
    QDomElement currentValue(root->firstChildElement("security").
                            firstChildElement("algorithm"));
    currentValue.removeChild(currentValue.firstChild());
    currentValue.appendChild(domDoc->createTextNode(alg));
    saveDoc();
}

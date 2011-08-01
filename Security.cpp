#include "Security.h"

Security::Security(const QString &key)
{
    init = new QCA::Initializer();
    if (key.length() != 32)
        this->key = new QCA::SymmetricKey(16);
    else
        this->key = new QCA::SymmetricKey(QCA::hexToArray(key));
    iv = new QCA::InitializationVector(QCA::hexToArray("f76a7571ebbdccd46175b2d53829ebb9"));
    cipher = new QCA::Cipher(QString("aes128"), QCA::Cipher::CBC,
                             QCA::Cipher::DefaultPadding, QCA::Encode,
                             *this->key, *iv);
}

/*
void encryptSomeData()
{
    //initialize QCA
    QCA::Initializer init = QCA::Initializer();
    //generate a random symmetric 16-bytes key
    QCA::SymmetricKey key = QCA::SymmetricKey(16);
    //generate a random 16-bytes initialization vector
    QCA::InitializationVector iv = QCA::InitializationVector(16);
    //initialize the cipher for aes128 algorithm, using CBC mode,
    //with padding enabled (by default), in encoding mode,
    //using the given key and initialization vector
    QCA::Cipher cipher = QCA::Cipher(QString("aes128"), QCA::Cipher::CBC,
                                     QCA::Cipher::DefaultPadding, QCA::Encode,
                                     key, iv);
    //check if aes128 is available
    if (!QCA::isSupported("aes128-cbc-pkcs7"))
    {
        qDebug() << "AES128 CBC PKCS7 not supported - "
                    "please check if qca-ossl plugin"
                    "installed correctly !";
        return;
    }
    //the string we want to encrypt
    QString s = "Hello, world !";
    //we use SecureArray: read more here:
    //http://delta.affinix.com/docs/qca/classQCA_1_1SecureArray.html#_details
    QCA::SecureArray secureData = s.toAscii();
    //we encrypt the data
    QCA::SecureArray result = cipher.process(secureData);
    //check if encryption succeded
    if (!cipher.ok())
    {
        qDebug() << "Encryption failed !";
        return;
    }
    //display the result
    qDebug() << QString(qPrintable(QCA::arrayToHex(result.toByteArray())));
    //set the cipher mode to encryption
    cipher.setup(QCA::Decode, key, iv);
    //decrypt the data
    result = cipher.process(secureData);
    //check if decryption succeded
    if (!cipher.ok())
    {
        qDebug() << "Decryption failed !";
        return "";
    }
    //display the decrypted data (it should be "Hello, world !")
    qDebug() << QString(result.data());
}*/

Security::~Security()
{
    delete key;
    delete cipher;
    delete init;
}

QString Security::encryptData(const QString &data) const
{
    if (!QCA::isSupported("aes128-cbc-pkcs7"))
    {
        qDebug() << "AES128 not supported";
        return "";
    }

    QCA::SecureArray dataToEncrypt = data.toAscii();
    cipher->setup(QCA::Encode, *key, *iv);

    QCA::SecureArray result = cipher->process(dataToEncrypt);
    if (!cipher->ok())
    {
        qDebug() << "Encryption failed\n";
        return "";
    }

    return QString(qPrintable(QCA::arrayToHex(result.toByteArray())));
}

QString Security::encryptData(const QString &data, const QString &key) const
{
    if (!QCA::isSupported("aes128-cbc-pkcs7"))
    {
        qDebug() << "AES128 not supported";
        return "";
    }

    QCA::SecureArray dataToEncrypt = data.toAscii();
    QCA::SymmetricKey k = QCA::SymmetricKey(QCA::hexToArray(key));
    cipher->setup(QCA::Encode, k, *iv);

    QCA::SecureArray result = cipher->process(dataToEncrypt);
    if (!cipher->ok())
    {
        qDebug() << "Encryption failed\n";
        return "";
    }

    return QString(qPrintable(QCA::arrayToHex(result.toByteArray())));
}

QString Security::decryptData(const QString &data) const
{
    if (!QCA::isSupported("aes128-cbc-pkcs7"))
    {
        qDebug() << "AES128 not supported";
        return "";
    }

    QCA::SecureArray dataToDecrypt = QCA::hexToArray(data);
    cipher->setup(QCA::Decode, *key, *iv);

    QCA::SecureArray result = cipher->process(dataToDecrypt);
    if (!cipher->ok())
    {
        qDebug() << "Decryption failed\n";
        return "";
    }

    return QString(result.data());
}

QString Security::decryptData(const QString &data, const QString &key) const
{
    if (!QCA::isSupported("aes128-cbc-pkcs7"))
    {
        qDebug() << "AES128 not supported";
        return "";
    }

    QCA::SecureArray dataToDecrypt = QCA::hexToArray(data);
    QCA::SymmetricKey k = QCA::SymmetricKey(QCA::hexToArray(key));
    cipher->setup(QCA::Decode, k, *iv);

    QCA::SecureArray result = cipher->process(dataToDecrypt);
    if (!cipher->ok())
    {
        qDebug() << "Decryption failed\n";
        return "";
    }

    return QString(result.data());
}

bool Security::loadKey(const QString &key)
{
    if (key.length() != 32)
        return false;

    this->key = new QCA::SymmetricKey(QCA::hexToArray(key));
    cipher->setup(QCA::Encode, *this->key, *iv);
    return true;
}

QString Security::generateKey()
{
    QCA::SymmetricKey newKey(16);
    return QString(qPrintable(QCA::arrayToHex(newKey.toByteArray())));
}

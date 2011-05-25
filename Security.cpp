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

QString Security::generateKey() const
{
    QCA::SymmetricKey newKey(16);
    return QString(qPrintable(QCA::arrayToHex(newKey.toByteArray())));
}

bool Security::loadKey(const QString &key)
{
    if (key.length() != 32)
        return false;

    this->key = new QCA::SymmetricKey(QCA::hexToArray(key));
    cipher->setup(QCA::Encode, *this->key, *iv);
    return true;
}

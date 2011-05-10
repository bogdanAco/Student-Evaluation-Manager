#ifndef SECURITY_H
#define SECURITY_H

#include <QtCrypto>
#include <QtCore>

class Security
{
public:
    Security(const QString &key);
    ~Security();
    QString encryptData(const QString &data);
    QString decryptData(const QString &data);
    QString generateKey();
    bool loadKey(const QString &key);

private:
    QCA::Initializer *init;
    QCA::SymmetricKey *key;
    QCA::InitializationVector *iv;
    QCA::Cipher *cipher;
};

#endif // SECURITY_H

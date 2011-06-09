#ifndef SECURITY_H
#define SECURITY_H

#include <QtCrypto>
#include <QtCore>

class Security
{
public:
    Security(const QString &key);
    ~Security();
    QString encryptData(const QString &data) const;
    QString encryptData(const QString &data,
                        const QString &key) const;
    QString decryptData(const QString &data) const;
    QString decryptData(const QString &data,
                        const QString &key) const;
    bool loadKey(const QString &key);

public slots:
    static QString generateKey();

private:
    QCA::Initializer *init;
    QCA::SymmetricKey *key;
    QCA::InitializationVector *iv;
    QCA::Cipher *cipher;
};

#endif // SECURITY_H

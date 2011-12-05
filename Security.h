#ifndef SECURITY_H
#define SECURITY_H

#include <QtCrypto>
#include <QtCore>

class Security
{
public:
    Security();
    ~Security();

    bool setAESkey(const QString &key);
    QString AESEncrypt(const QString &data) const;
    QString AESDecrypt(const QString &data) const;

    bool setRSAkeys(const QString &pubkey,
                    const QString &prvkey,
                    const QString &passphrase);
    QString RSAEncrypt(const QString &data) const;
    QString RSADecrypt(const QString &data) const;

    static QString AESEncrypt(const QString &data,
                              const QString &key);
    static QString AESDecrypt(const QString &data,
                              const QString &key);
    static QString RSAEncrypt(const QString &data,
                              const QString &pubkey);
    static QString RSADecrypt(const QString &data,
                              const QString &prvkey);
    static QString getHash(const QString &text);
    static QString generateAESKey();
    static QPair<QString, QString>
        generateKeyPair(const QString &passphrase);

private:
    QCA::Initializer *init;
    QCA::SymmetricKey *AESkey;
    QCA::InitializationVector *AESiv;
    QCA::Cipher *AEScipher;
    QCA::PublicKey *RSApublic;
    QCA::PrivateKey *RSAprivate;
};

#endif // SECURITY_H

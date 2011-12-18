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
    QString getAESkey();
    QString AESEncrypt(const QString &data) const;
    QString AESDecrypt(const QString &data) const;

    bool setRSAkeys(const QString &pubKeyData,
                    const QString &prvKeyData,
                    const QString &passphrase);
    QString RSAEncrypt(const QString &data) const;
    QString RSADecrypt(const QString &data) const;
    QString RSASign(const QString &data) const;

    static QString AESEncrypt(const QString &data,
                              const QString &key);
    static QString AESDecrypt(const QString &data,
                              const QString &key);
    static QString RSAEncrypt(const QString &data,
                              const QString &pubkeyData);
    static QString RSADecrypt(const QString &data,
                              const QString &prvkeyData,
                              const QString &passphrase);
    static bool RSAVerifySignature(const QString &data,
                            const QString &signedData,
                            const QString &pubKeyData);
    static QString getHash(const QString &text);
    static QString generateAESKey();
    static QPair<QString, QString>
        generateKeyPair(const QString &passphrase);

private:
    QCA::Initializer *init;
    QCA::SymmetricKey *AESkey;
    QCA::InitializationVector *AESiv;
    QCA::Cipher *AEScipher;
    QCA::RSAPublicKey *RSApublic;
    QCA::RSAPrivateKey *RSAprivate;
};

#endif // SECURITY_H

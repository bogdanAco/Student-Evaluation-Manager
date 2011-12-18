#include "Security.h"

Security::Security()
{
    init = new QCA::Initializer();
    AESkey = 0;
    AESiv = new QCA::InitializationVector(QCA::hexToArray("f76a7571ebbdccd46175b2d53829ebb9"));
    AEScipher = 0;
    RSApublic = 0;
    RSAprivate = 0;
}

Security::~Security()
{
    delete AESkey;
    delete AESiv;
    delete AEScipher;
    delete RSApublic;
    delete RSAprivate;
    delete init;
}

bool Security::setAESkey(const QString &key)
{
    if (key.length() != 64 || !QCA::isSupported("aes256-cbc-pkcs7"))
        return false;

    delete AEScipher;
    AESkey = new QCA::SymmetricKey(QCA::hexToArray(key));
    AEScipher = new QCA::Cipher(QString("aes256"), QCA::Cipher::CBC,
                                QCA::Cipher::DefaultPadding, QCA::Encode,
                                *AESkey, *AESiv);
    
    return true;
}

QString Security::getAESkey()
{
    if (AESkey == 0)
        return "";
    
    return QString(qPrintable(QCA::arrayToHex(AESkey->toByteArray())));
}

QString Security::AESEncrypt(const QString &data) const
{
    if (!QCA::isSupported("aes256-cbc-pkcs7"))
        return "";
    if (AESkey == 0 || AEScipher == 0)
        return "";

    QCA::SecureArray dataToEncrypt = data.toAscii();
    AEScipher->setup(QCA::Encode, *AESkey, *AESiv);

    QCA::SecureArray result = AEScipher->process(dataToEncrypt);
    if (!AEScipher->ok())
        return "";
    
    return QString(qPrintable(QCA::arrayToHex(result.toByteArray())));
}

QString Security::AESDecrypt(const QString &data) const
{
    if (!QCA::isSupported("aes256-cbc-pkcs7"))
        return "";
    if (AESkey == 0 || AEScipher == 0)
        return "";

    QCA::SecureArray dataToDecrypt = QCA::hexToArray(data);
    AEScipher->setup(QCA::Decode, *AESkey, *AESiv);

    QCA::SecureArray result = AEScipher->process(dataToDecrypt);
    if (!AEScipher->ok())
        return "";
    
    return QString(result.data());
}

bool Security::setRSAkeys(const QString &pubKeyData, 
                          const QString &prvKeyData, 
                          const QString &passphrase)
{
    if (passphrase.length() != 64 || !QCA::isSupported("pkey"))
        return false;

    delete this->RSApublic;
    delete this->RSAprivate;
    
    QStringList pubKeyParam = pubKeyData.split('|');
    QStringList prvKeyParam = AESDecrypt(prvKeyData, passphrase).split('|');
    
    if (pubKeyParam.length() != 2 || prvKeyParam.length() != 5)
        return false;
    
    RSApublic = new QCA::RSAPublicKey(pubKeyParam.at(0), 
                                      pubKeyParam.at(1));
    RSAprivate = new QCA::RSAPrivateKey(prvKeyParam.at(0), 
                                        prvKeyParam.at(1),
                                        prvKeyParam.at(2),
                                        prvKeyParam.at(3),
                                        prvKeyParam.at(4));
    return true;
}

QString Security::RSAEncrypt(const QString &data) const
{
    if (!QCA::isSupported("pkey"))
        return "";
    if (RSApublic == 0)
        return "";
    if (!RSApublic->canEncrypt())
        return "";
    
    QCA::SecureArray dataToEncrypt = data.toAscii();
    QCA::SecureArray result = RSApublic->encrypt(dataToEncrypt, QCA::EME_PKCS1_OAEP);

    return QString(qPrintable(QCA::arrayToHex(result.toByteArray())));
}

QString Security::RSADecrypt(const QString &data) const
{
    if (!QCA::isSupported("pkey"))
        return "";
    if (RSAprivate == 0)
        return "";
    if (!RSAprivate->canDecrypt())
        return "";

    QCA::SecureArray dataToDecrypt = QCA::hexToArray(data);
    QCA::SecureArray result;
    if (RSAprivate->decrypt(dataToDecrypt, &result, QCA::EME_PKCS1_OAEP))
        return QString(result.data());
    else
        return "";
}

QString Security::RSASign(const QString &data) const
{
    if (!QCA::isSupported("pkey"))
        return "";
    if (RSAprivate == 0)
        return "";
    if (!RSAprivate->canSign())
        return "";
    
    RSAprivate->startSign(QCA::EMSA3_SHA1);
    RSAprivate->update(QCA::SecureArray(QCA::hexToArray(data)));
    return QString(qPrintable(QCA::arrayToHex(RSAprivate->signature())));
}

QString Security::AESEncrypt(const QString &data, const QString &key)
{
    QCA::Initializer init;
    if (!QCA::isSupported("aes256-cbc-pkcs7") || key.length() != 64)
        return "";

    QCA::InitializationVector iv(QCA::hexToArray("f76a7571ebbdccd46175b2d53829ebb9"));
    QCA::SymmetricKey k(QCA::hexToArray(key));
    QCA::SecureArray dataToEncrypt = data.toAscii();
    QCA::Cipher cipher = QCA::Cipher(QString("aes256"), QCA::Cipher::CBC,
            QCA::Cipher::DefaultPadding, QCA::Encode, k, iv);

    QCA::SecureArray result = cipher.process(dataToEncrypt);
    if (!cipher.ok())
        return "";

    return QString(qPrintable(QCA::arrayToHex(result.toByteArray())));
}

QString Security::AESDecrypt(const QString &data, const QString &key)
{
    QCA::Initializer init;
    if (!QCA::isSupported("aes256-cbc-pkcs7") || key.length() != 64)
        return "";

    QCA::InitializationVector iv(QCA::hexToArray("f76a7571ebbdccd46175b2d53829ebb9"));
    QCA::SymmetricKey k(QCA::hexToArray(key));
    QCA::SecureArray dataToDecrypt = QCA::hexToArray(data);
    QCA::Cipher cipher = QCA::Cipher(QString("aes256"), QCA::Cipher::CBC,
            QCA::Cipher::DefaultPadding, QCA::Decode, k, iv);

    QCA::SecureArray result = cipher.process(dataToDecrypt);
    if (!cipher.ok())
        return "";

    return QString(result.data());
}

QString Security::RSAEncrypt(const QString &data, const QString &pubkey)
{
    QCA::Initializer init;
    if (!QCA::isSupported("pkey"))
        return "";

    QStringList pubKeyParam = pubkey.split('|');
    if (pubKeyParam.length() != 2)
        return "";
    
    QCA::RSAPublicKey key = QCA::RSAPublicKey(pubKeyParam.at(0), 
                                              pubKeyParam.at(1));
    if (!key.canEncrypt())
        return "";

    QCA::SecureArray dataToEncrypt = data.toAscii();
    QCA::SecureArray result = key.encrypt(dataToEncrypt, QCA::EME_PKCS1_OAEP);

    return QString(qPrintable(QCA::arrayToHex(result.toByteArray())));
}

QString Security::RSADecrypt(const QString &data, const QString &prvkey, const QString &passphrase)
{
    QCA::Initializer init;
    if (passphrase.length() != 64 || !QCA::isSupported("pkey"))
        return "";

    QStringList prvKeyParam = AESDecrypt(prvkey, passphrase).split('|');
    if (prvKeyParam.length() != 5)
        return "";

    QCA::RSAPrivateKey key = QCA::RSAPrivateKey(prvKeyParam.at(0), 
                                                prvKeyParam.at(1),
                                                prvKeyParam.at(2),
                                                prvKeyParam.at(3),
                                                prvKeyParam.at(4));
    if (!key.canDecrypt())
        return "";

    QCA::SecureArray dataToDecrypt = QCA::hexToArray(data);
    QCA::SecureArray result;
    if (key.decrypt(dataToDecrypt, &result, QCA::EME_PKCS1_OAEP))
        return QString(result.data());
    else
        return "";
}

bool Security::RSAVerifySignature(const QString &data, 
                                  const QString &signedData, 
                                  const QString &pubKeyData)
{
    QCA::Initializer init;
    if (!QCA::isSupported("pkey"))
        return "";
    
    QStringList pubKeyParam = pubKeyData.split('|');
    if (pubKeyParam.length() != 2)
        return "";
    
    QCA::RSAPublicKey key = QCA::RSAPublicKey(pubKeyParam.at(0), 
                                              pubKeyParam.at(1));
    if (!key.canEncrypt())
        return "";
    
    key.startVerify(QCA::EMSA3_SHA1);
    key.update(QCA::SecureArray(QCA::hexToArray(data)));
    return key.validSignature(QCA::hexToArray(signedData));
}

QString Security::getHash(const QString &text)
{
    QCA::Initializer init;
    if(!QCA::isSupported("sha256"))
        return "";

    QCA::Hash hash = QCA::Hash("sha256");
    hash.update(text.toAscii());
    return hash.hashToString(hash.final());
}

QString Security::generateAESKey()
{
    QCA::Initializer init;
    if (!QCA::isSupported("aes256-cbc-pkcs7"))
        return "";

    QCA::SymmetricKey rndkey(32);
    return QString(qPrintable(QCA::arrayToHex(rndkey.toByteArray())));
}

QPair<QString,QString> Security::generateKeyPair(const QString &passphrase)
{
    QCA::Initializer init;
    if(!QCA::isSupported("pkey") ||
           !QCA::PKey::supportedIOTypes().contains(QCA::PKey::RSA))
        return QPair<QString,QString>();

    QCA::RSAPrivateKey prvkey = QCA::KeyGenerator().createRSA(1024).toRSA();
    QCA::RSAPublicKey pubkey = prvkey.toPublicKey().toRSA();
    QString pubkeyData = QString("%1|%2").
            arg(pubkey.n().toString()).
            arg(pubkey.e().toString());
    QString prvkeyData = QString("%1|%2|%3|%4|%5").
            arg(pubkey.n().toString()).
            arg(pubkey.e().toString()).
            arg(prvkey.p().toString()).
            arg(prvkey.q().toString()).
            arg(prvkey.d().toString());
    
    return QPair<QString, QString>(pubkeyData, AESEncrypt(prvkeyData, passphrase));
}

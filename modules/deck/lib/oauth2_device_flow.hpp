#pragma once

//Libs
#include <QNetworkAccessManager>
#include <QObject>
#include <QTimer>
#include <QUrl>

namespace Ars::Deck {
class OAuth2DeviceFlow : public QObject {
    Q_OBJECT

public:
    explicit OAuth2DeviceFlow(QObject* parent = nullptr);

    void grant();

    //Getters & Setters
    QString getClientId() const { return m_clientId; }
    void setClientId(const QString& clientId) { m_clientId = clientId; }

    QUrl getDeviceUrl() const { return m_deviceUrl; }
    void setDeviceUrl(const QUrl& deviceUrl) { m_deviceUrl = deviceUrl; };

    QUrl getTokenUrl() const { return m_tokenUrl; }
    void setTokenUrl(const QUrl& tokenUrl) { m_tokenUrl = tokenUrl; }

    QString getDeviceCode() const { return m_deviceCode; }
    void setDeviceCode(const QString& deviceCode) { m_deviceCode = deviceCode; }

    QString getToken() const { return m_token; }
    void setToken(const QString& token) { m_token = token; }

signals:
    void granted();
    void authorizeWithUserCode(const QUrl& verificationUrl, const QString& userCode, const QUrl& completeVerificationUrl);
    void errorOccured(const QString& error);

private slots:
    void pollToken();

private:
    QTimer* m_pollTimer;
    QNetworkAccessManager* m_networkAccessManager;

    QString m_clientId;
    QUrl m_deviceUrl;
    QUrl m_tokenUrl;

    QString m_deviceCode;
    QString m_token;
};
}  // namespace Ars::Deck

//Libs
#include <QDBusReply>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>

#include "auth_manager.hpp"

const QString SONAR_SERVICE_NAME = "org.ars.sonar";
const QString SONAR_OBJECT_PATH = "/org/ars/sonar";
const QString SONAR_INTERFACE_NAME = "org.ars.sonar.Interface";

namespace Ars::Deck {
AuthManager::AuthManager(QObject* parent) : QObject(parent), m_oauth(this) {
    m_dbusInterface = new QDBusInterface(SONAR_SERVICE_NAME, SONAR_OBJECT_PATH, SONAR_INTERFACE_NAME, QDBusConnection::sessionBus(), this);

    if (!m_dbusInterface->isValid()) {
        qWarning() << "D-Bus interface is NOT valid:" << SONAR_SERVICE_NAME;
    }
    this->setupGithub();

    connect(&m_oauth, &OAuth2DeviceFlow::authorizeWithUserCode, this,
            [this](const QUrl& verificationUrl, const QString& userCode, const QUrl& completeVerificationUrl) {
                qInfo() << "Code received:" << userCode;
                emit deviceAuthReady(verificationUrl.toString(), userCode);
            });

    connect(&m_oauth, &OAuth2DeviceFlow::granted, this, &AuthManager::onAuthFinished);
}

void AuthManager::startAuth() {
    if (m_oauth.getClientId().isEmpty()) {
        qWarning() << "Cannot start auth: Client ID is not set";
        emit tokenErrorReceived("Client ID missing");
        return;
    }
    m_oauth.grant();
}

void AuthManager::setupGithub() {
    m_oauth.setDeviceUrl(QUrl("https://github.com/login/device/code"));
    m_oauth.setTokenUrl(QUrl("https://github.com/login/oauth/access_token"));
    m_oauth.setClientId("Ov23liaSalefAhl16gjU");
}

// TODO: Gitlab fetch method
void AuthManager::setupGitlab() {
    // Not implemented yet
}

void AuthManager::onAuthFinished() {
    QString token = m_oauth.getToken();
    qInfo() << "Authentication Successful!";
    qInfo() << "Access Token:" << token;

    QDBusReply<bool> reply = m_dbusInterface->call("UpdateToken", "github", token);
    if (reply.isValid() && reply.value()) {
        qInfo() << "Token successfully sent to daemon";
    } else {
        qWarning() << "Failed to send token: " << reply.error().message();
    }
    emit tokenReceived(token);
}

}  // namespace Ars::Deck

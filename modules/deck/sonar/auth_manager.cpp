#include "auth_manager.hpp"
#include <QDBusReply>
#include <QDebug>
#include <QDesktopServices>
#include <QOAuth2DeviceAuthorizationFlow>
#include <QUrl>
#include "utils/json_network_manager.hpp"

const QString SONAR_SERVICE_NAME = "org.ars.sonar";
const QString SONAR_OBJECT_PATH = "/org/ars/sonar";
const QString SONAR_INTERFACE_NAME = "org.ars.sonar.Interface";

namespace Ars::Deck {
AuthManager::AuthManager(QObject* parent) : QObject(parent), m_oauth(new QOAuth2DeviceAuthorizationFlow(this)) {
    m_dbusInterface = new QDBusInterface(SONAR_SERVICE_NAME, SONAR_OBJECT_PATH, SONAR_INTERFACE_NAME, QDBusConnection::sessionBus(), this);

    if (!m_dbusInterface->isValid()) {
        qWarning() << "D-Bus interface is NOT valid:" << SONAR_SERVICE_NAME;
    }
    m_oauth->setNetworkAccessManager(new JsonNetworkManager(this));
    this->setupGithub();

    connect(m_oauth, &QOAuth2DeviceAuthorizationFlow::authorizeWithUserCode, this,
            [this](const QUrl& verificationUrl, const QString& userCode, const QUrl& completeVerificationUrl) {
                qInfo() << "Code received:" << userCode;
                emit deviceAuthReady(verificationUrl.toString(), userCode);
            });

    connect(m_oauth, &QAbstractOAuth::granted, this, &AuthManager::onAuthFinished);
}

void AuthManager::startAuth() {
    if (m_oauth->clientIdentifier().isEmpty()) {
        qWarning() << "Cannot start auth: Client ID is not set";
        emit tokenErrorReceived("Client ID missing");
        return;
    }
    m_oauth->grant();
}

void AuthManager::setupGithub() {
    m_oauth->setAuthorizationUrl(QUrl("https://github.com/login/device/code"));
    m_oauth->setTokenUrl(QUrl("https://github.com/login/oauth/access_token"));
    m_oauth->setClientIdentifier("Ov23liaSalefAhl16gjU");
    m_oauth->setRequestedScopeTokens({"repo", "user"});
}

// TODO: Gitlab fetch method
void AuthManager::setupGitlab() {
    // Not implemented yet
}

void AuthManager::onAuthFinished() {
    if (m_oauth->status() == QAbstractOAuth::Status::Granted) {
        QString token = m_oauth->token();
        qInfo() << "Authentication Successful!";
        qInfo() << "Access Token:" << token;

        QDBusReply<bool> reply = m_dbusInterface->call("UpdateToken", "github", token);
        if (reply.isValid() && reply.value()) {
            qInfo() << "Token successfully sent to daemon";
        } else {
            qWarning() << "Failed to send token: " << reply.error().message();
        }
        emit tokenReceived(token);
    } else {
        qWarning() << "Auth finished but status is not Granted";
        emit tokenErrorReceived("Auth failed or rejected");
    }
}

}  // namespace Ars::Deck

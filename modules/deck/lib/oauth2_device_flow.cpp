#include "oauth2_device_flow.hpp"
#include "json_network_manager.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QUrlQuery>

namespace Ars::Deck {

OAuth2DeviceFlow::OAuth2DeviceFlow(QObject* parent)
    : QObject(parent), m_networkAccessManager(new JsonNetworkManager(this)), m_pollTimer(new QTimer(this)) {
    connect(m_pollTimer, &QTimer::timeout, this, &OAuth2DeviceFlow::pollToken);
}

void OAuth2DeviceFlow::grant() {
    QNetworkRequest request(m_deviceUrl);
    QUrlQuery urlQuery;

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    urlQuery.addQueryItem("client_id", m_clientId);
    QByteArray clientId(urlQuery.toString(QUrl::FullyEncoded).toUtf8());

    QNetworkReply* reply = m_networkAccessManager->post(request, clientId);

    connect(reply, &QNetworkReply::finished, reply, [reply, this]() {
        if (!reply->error()) {
            QByteArray result = reply->readAll();
            QJsonDocument doc;
            doc = QJsonDocument::fromJson(result);

            QString userCode = doc["user_code"].toString();
            QString verificationUri = doc["verification_uri"].toString();
            qint16 interval = doc["interval"].toInt();

            m_deviceCode = doc["device_code"].toString();

            emit authorizeWithUserCode(QUrl(verificationUri), userCode, QUrl());

            m_pollTimer->start((interval + 1) * 1000);
        } else {
            emit errorOccured(reply->errorString());
        }

        reply->deleteLater();
    });
}

void OAuth2DeviceFlow::pollToken() {
    QNetworkRequest request(m_tokenUrl);
    QUrlQuery urlQuery;

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    urlQuery.addQueryItem("client_id", m_clientId);
    urlQuery.addQueryItem("scope", "repo user");
    urlQuery.addQueryItem("device_code", m_deviceCode);
    urlQuery.addQueryItem("grant_type", "urn:ietf:params:oauth:grant-type:device_code");

    QByteArray result(urlQuery.toString(QUrl::FullyEncoded).toUtf8());

    QNetworkReply* reply = m_networkAccessManager->post(request, result);

    connect(reply, &QNetworkReply::finished, reply, [this, reply]() {
        if (!reply->error()) {
            QByteArray result = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(result);
            QJsonObject json = doc.object();

            if (json.contains("access_token")) {
                m_token = json["access_token"].toString();
                m_pollTimer->stop();
                emit granted();
            } else if (json.contains("error")) {
                QString errorType = json["error"].toString();

                if (errorType == "authorization_pending") {

                } else if (errorType == "slow_down") {
                    int newInterval = json["interval"].toInt();
                    m_pollTimer->setInterval((newInterval + 1) * 1000);
                } else if (errorType == "expired_token") {
                    m_pollTimer->stop();
                    emit errorOccured("Device code expired. Please try again.");
                }
            }
        }

        reply->deleteLater();
    });
}
}  // namespace Ars::Deck

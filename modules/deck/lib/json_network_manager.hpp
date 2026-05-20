#pragma once

//Libs
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace Ars::Deck {
class JsonNetworkManager : public QNetworkAccessManager {
    Q_OBJECT
public:
    explicit JsonNetworkManager(QObject* parent = nullptr) : QNetworkAccessManager(parent) {}

protected:
    QNetworkReply* createRequest(Operation operation, const QNetworkRequest& request, QIODevice* outgoingData = nullptr) override {
        QNetworkRequest newRequest = request;

        newRequest.setRawHeader("Accept", "application/json");
        newRequest.setRawHeader("User-Agent", "Deck");
        newRequest.setRawHeader("X-GitHub-Api-Version", "2022-11-28");

        QNetworkReply* reply = QNetworkAccessManager::createRequest(operation, newRequest, outgoingData);
        connect(reply, &QNetworkReply::finished, reply, [reply]() {
            qDebug() << "--- Network Request ---";
            qDebug() << "URL:" << reply->url().toString();
            qDebug() << "Status:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            QByteArray data = reply->peek(reply->bytesAvailable());
            qDebug() << "Body:" << data;
            qDebug() << "-----------------------";
        });

        return reply;
    }

private:
};

}  // namespace Ars::Deck

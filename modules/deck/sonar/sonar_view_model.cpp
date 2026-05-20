//Libs
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>

#include "sonar_view_model.hpp"

const QString SONAR_SERVICE_NAME = "org.ars.sonar";
const QString SONAR_OBJECT_PATH = "/org/ars/sonar";
const QString SONAR_INTERFACE_NAME = "org.ars.sonar.Interface";

namespace Ars::Deck {
SonarViewModel::SonarViewModel(QObject* parent) {
    m_dbusInterface = new QDBusInterface(SONAR_SERVICE_NAME, SONAR_OBJECT_PATH, SONAR_INTERFACE_NAME, QDBusConnection::sessionBus(), this);

    if (!m_dbusInterface->isValid()) {
        qWarning() << "D-Bus interface is NOT valid:" << SONAR_SERVICE_NAME;
    }
}

void SonarViewModel::fetchPipelineData() {
    qInfo() << "QML button clicked, calling D-Bus method FetchData...";

    QDBusPendingCall asyncCall = m_dbusInterface->asyncCall("PipelineStatusFetch");
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(asyncCall, this);

    connect(watcher, &QDBusPendingCallWatcher::finished, this, &SonarViewModel::onDataFetchFinished);
}

void SonarViewModel::onDataFetchFinished(QDBusPendingCallWatcher* watcher) {
    QDBusPendingReply<QString> reply = *watcher;

    if (reply.isValid()) {
        QByteArray result = reply.value().toUtf8();
        qInfo() << "D-Bus Reply OK:";
        qInfo() << "RAW JSON:" << result;

        beginResetModel();
        m_pipeline_data.clear();

        QJsonDocument doc;
        doc = QJsonDocument::fromJson(result);

        if (!doc.isArray()) {
            qWarning() << "Unvalid Json";
            endResetModel();
            return;
        } else {
            QJsonArray json_array = doc.array();

            for (auto pipeline : json_array) {
                PipelineData pipeline_data;
                QJsonObject object = pipeline.toObject();

                pipeline_data.m_id = object["id"].toInteger();
                pipeline_data.m_pipeline_name = object["name"].toString();
                pipeline_data.m_status = object["status"].toString();
                pipeline_data.m_conclusion = object["conclusion"].toString();

                m_pipeline_data.push_back(pipeline_data);
            }
        }
        endResetModel();

        emit pipelineDataReceived(result);
    } else {
        QString error = reply.error().message();
        qWarning() << "D-Bus Call Failed:" << error;
        emit pipelineDataErrorReceived(error);
    }

    watcher->deleteLater();
}
}  // namespace Ars::Deck

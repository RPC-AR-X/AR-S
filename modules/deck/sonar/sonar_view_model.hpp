#pragma once

//Libs
#include <QAbstractListModel>
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDebug>
#include <QObject>
#include <Qt>

namespace Ars::Deck {

struct PipelineData {
    qint64 m_id;
    QString m_status;
    QString m_pipeline_name;
    QString m_conclusion;
};

enum Roles { IdRole = Qt::UserRole + 1, NameRole, StatusRole, ConclusionRole };

class SonarViewModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit SonarViewModel(QObject* parent = nullptr);

    virtual int rowCount(const QModelIndex& parent) const override { return m_pipeline_data.size(); };
    virtual QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid() || index.row() >= m_pipeline_data.size()) {
            return QVariant();
        }

        const PipelineData& item = m_pipeline_data[index.row()];

        switch (role) {
            case IdRole:
                return item.m_id;
            case NameRole:
                return item.m_pipeline_name;
            case StatusRole:
                return item.m_status;
            case ConclusionRole:
                return item.m_conclusion;
        }

        return QVariant();
    };
    virtual QHash<int, QByteArray> roleNames() const override {
        QHash<int, QByteArray> roles;

        roles[IdRole] = "id";
        roles[NameRole] = "name";
        roles[StatusRole] = "status";
        roles[ConclusionRole] = "conclusion";

        return roles;
    }

signals:
    void pipelineDataReceived(const QString& data);
    void pipelineDataErrorReceived(const QString& error);

public slots:
    void fetchPipelineData();

private slots:
    void onDataFetchFinished(QDBusPendingCallWatcher* watcher);

private:
    QDBusInterface* m_dbusInterface;
    QVector<PipelineData> m_pipeline_data;
};
}  // namespace Ars::Deck

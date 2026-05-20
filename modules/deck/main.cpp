//Libs
#include <QDBusConnection>
#include <QDBusInterface>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "sonar/auth_manager.hpp"
#include "sonar/sonar_view_model.hpp"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    Ars::Deck::AuthManager authManager;
    authManager.setupGithub();

    Ars::Deck::SonarViewModel sonar_view_model;

    engine.rootContext()->setContextProperty("SonarViewModel", &sonar_view_model);
    engine.rootContext()->setContextProperty("AuthManager", &authManager);

    qmlRegisterSingletonType(QUrl("qrc:/Theme.qml"), "Ars.Theme", 1, 0, "Theme");

    const QUrl url(QStringLiteral("qrc:/Main.qml"));

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
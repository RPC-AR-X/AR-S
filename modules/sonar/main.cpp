//Standard Includes
#include <signal.h>
#include <csignal>
#include <exception>
#include <iostream>
#include <memory>
#include <thread>

//Libs
#include <sdbus-c++/IConnection.h>
#include <sdbus-c++/IObject.h>
#include <sdbus-c++/Types.h>

#include <adapters/dbus_adapter.h>

int main() {

    int signal;
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);

    pthread_sigmask(SIG_BLOCK, &set, nullptr);

    try {
        sdbus::ServiceName serviceName{"org.ars.sonar"};
        auto connection = sdbus::createSessionBusConnection(serviceName);

        sdbus::ObjectPath objectPath{"/org/ars/sonar"};
        DbusAdapter adapter(*connection, std::move(objectPath));

        std::thread eventLoopThread([&]() { connection->enterEventLoop(); });
        sigwait(&set, &signal);

        adapter.emitSonarStopWorkSignal("Sonar exit gracefully...");
        std::cout << "Signal emitted..." << std::endl;

        connection->leaveEventLoop();

        if (eventLoopThread.joinable()) {
            eventLoopThread.join();
        }

    } catch (std::exception& e) {
        std::cout << "ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

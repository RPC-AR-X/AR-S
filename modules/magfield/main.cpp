#include <iostream>
#include <vector>

#include <sdbus-c++/IConnection.h>
#include <sdbus-c++/Message.h>
#include <sdbus-c++/Types.h>
#include <sdbus-c++/sdbus-c++.h>

int main() {

    auto connection = sdbus::createSystemBusConnection();

    sdbus::ServiceName networkManagerDestination{"org.freedesktop.NetworkManager"};
    sdbus::ObjectPath networkManagerObjectPath{"/org/freedesktop/NetworkManager"};

    auto networkManagerProxy = sdbus::createProxy(*connection, networkManagerDestination, std::move(networkManagerObjectPath));

    sdbus::InterfaceName interfaceName{"org.freedesktop.NetworkManager"};

    sdbus::MethodName getDevices{"GetDevices"};
    auto method = networkManagerProxy->createMethodCall(interfaceName, getDevices);
    auto reply = networkManagerProxy->callMethod(method);

    std::vector<sdbus::ObjectPath> devices;
    reply >> devices;

    if (!devices.empty()) {

        for (auto& device : devices) {
            sdbus::ObjectPath devicesObjectPath{device};
            auto currentConnectionProxy = sdbus::createProxy(*connection, networkManagerDestination, std::move(devicesObjectPath));

            auto currentConnectionInfo = currentConnectionProxy->getProperty("DeviceType").onInterface("org.freedesktop.NetworkManager.Device");
            auto deviceType = currentConnectionInfo.get<unsigned int>();

            if (deviceType == 1) {
                std::cout << "Ethernet" << std::endl;
            } else if (deviceType == 2) {
                std::cout << "Wifi" << std::endl;
            }
        }
    }

    return 0;
}
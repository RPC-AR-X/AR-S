//Libs
#include <nlohmann/json.hpp>

#include "adapters/dbus_adapter.h"
#include "providers/github_provider.h"

DbusAdapter::DbusAdapter(sdbus::IConnection& connection, sdbus::ObjectPath objectPath) : AdaptorInterfaces(connection, std::move(objectPath)) {
    std::cout << "DbusAdapter constructor" << std::endl;
    providers_.push_back(std::make_unique<GitHubProvider>());

    registerAdaptor();
}

DbusAdapter::~DbusAdapter() {
    std::cout << "DbusAdapter destructor" << std::endl;
    unregisterAdaptor();
}

std::string DbusAdapter::PipelineStatusFetch() {
    using namespace std::chrono;
    std::cout << "PipelineStatusFetch called" << std::endl;

    nlohmann::json results_array = nlohmann::json::array();

    for (const auto& provider : providers_) {
        auto cpu_start = high_resolution_clock::now();

        std::string provider_json_string = provider->FetchStatusAsJson();

        auto cpu_end = high_resolution_clock::now();
        auto cpu_duration_us = duration_cast<microseconds>(cpu_end - cpu_start).count();
        std::cout << "[TIMING] CPU-bound (adapter dispatch + polymorphism): " << cpu_duration_us << " us" << std::endl;

        if (!provider_json_string.empty()) {
            try {
                auto provider_json = nlohmann::json::parse(provider_json_string);

                if (provider_json.is_array()) {
                    for (const auto& item : provider_json) {
                        results_array.push_back(item);
                    }
                } else {
                    results_array.push_back(provider_json);
                }
            } catch (const nlohmann::json::exception& e) {
                std::cerr << "JSON Merge Error: " << e.what() << std::endl;
            }
        }
    }

    return results_array.dump(4);
}

bool DbusAdapter::UpdateToken(const std::string& providerName, const std::string& token) {
    std::cout << providerName << std::endl;

    for (const auto& provider : providers_) {
        if (provider->GetProviderName() == providerName) {
            provider->SetToken(token);
            return true;
        }
    }

    std::cerr << "Error: Provider not found: " << providerName << std::endl;
    return false;
}

void DbusAdapter::emitSonarStopWorkSignal(const std::string& message) {
    emitSonarWorkStop(message);
}
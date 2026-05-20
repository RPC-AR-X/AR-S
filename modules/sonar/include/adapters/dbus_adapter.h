#pragma once

//Standard Includes
#include <memory>
#include <vector>

#include <providers/ipipeline_provider.h>

#include "PipelineAdaptor.h"

class DbusAdapter final : public sdbus::AdaptorInterfaces<org::ars::sonar::Interface_adaptor> {
public:
    DbusAdapter(sdbus::IConnection& connection, sdbus::ObjectPath objectPath);
    ~DbusAdapter();

    std::string PipelineStatusFetch() override;
    bool UpdateToken(const std::string& providerName, const std::string& token) override;

    void emitSonarStopWorkSignal(const std::string& message);

private:
    std::vector<std::unique_ptr<IPipelineProvider>> providers_;
};
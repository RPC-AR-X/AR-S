#pragma once

//Standard Includes
#include <fcntl.h>
#include <pty.h>

//gRPC Includes
#include <grpcpp/grpcpp.h>
#include "agent.grpc.pb.h"
#include "agent.pb.h"

#include "shell_controller.hh"
#include "fd_handler.hh"

namespace Ars::Controller::Shell {
class AbortedReactor : public grpc::ServerBidiReactor<Agent::ExecuteShellRequest, Agent::ExecuteShellResponse> {
public:
    explicit AbortedReactor(const grpc::Status& status) { Finish(status); }

    void OnDone() override { delete this; }
    void OnReadDone(bool ok) override {}
    void OnWriteDone(bool ok) override {}
};

class ProcessingImplementation : public Agent::ShellControllerService::CallbackService {
public:
    grpc::ServerBidiReactor<Agent::ExecuteShellRequest, Agent::ExecuteShellResponse>* ExecuteShell(grpc::CallbackServerContext* context) override {
        try {
            int master_fd;
            pid_t pid = forkpty(&master_fd, nullptr, nullptr, nullptr);

            if (pid < 0) {
                return new AbortedReactor(grpc::Status(grpc::StatusCode::INTERNAL, "forkpty failed"));
            }

            if (pid == 0) {
                execlp("bash", "bash", nullptr);
                exit(1);
            }

            FDHandler fd_handler(master_fd);

            auto* reactor = new ShellReactor(std::move(fd_handler), pid);
            reactor->Start();

            return reactor;
        } catch (const std::exception& e) {
            std::cerr << "ShellReactor creation failed: " << e.what() << std::endl;
            return new AbortedReactor(grpc::Status(grpc::StatusCode::INTERNAL, e.what()));
        }
    }
};
} // namespace Ars::Controller::Shell
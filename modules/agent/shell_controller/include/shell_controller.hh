#pragma once

//Standard Includes
#include <fcntl.h>
#include <pty.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include <array>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

//gRPC Includes
#include <grpcpp/grpcpp.h>
#include "agent.grpc.pb.h"
#include "agent.pb.h"

namespace Ars::Controller::Shell {
class ShellReactor : public grpc::ServerBidiReactor<Agent::ExecuteShellRequest, Agent::ExecuteShellResponse> {
public:
    using OutputCallback = std::function<void(const std::string&)>;

    explicit ShellReactor(int master_fd, pid_t pid, OutputCallback output_callback = nullptr);

    Agent::ExecuteShellRequest* GetRequest() { return &request_; }

    void Start() { StartRead(&request_); }

    void OnReadDone(bool ok) override;
    void OnWriteDone(bool ok) override;

    void OnDone() override {
        running_ = false;
        close(master_fd_);

        if (epoll_cycle.joinable()) {
            epoll_cycle.join();
        }

        waitpid(pid_, nullptr, 0);
        close(epoll_fd_);

        delete this;
    }

private:
    void DoNextWrite();

    int master_fd_;
    pid_t pid_;
    int epoll_fd_;

    std::thread epoll_cycle;
    std::mutex queue_mutex_;
    std::atomic<bool> running_{true};
    std::atomic_flag writing_in_progress_;
    std::condition_variable data_notifier_;
    std::queue<std::string> shell_output_queue_;

    OutputCallback output_callback_;

    Agent::ExecuteShellRequest request_;
    Agent::ExecuteShellResponse response_;
};

class AbortedReactor : public grpc::ServerBidiReactor<Agent::ExecuteShellRequest, Agent::ExecuteShellResponse> {
public:
    explicit AbortedReactor(const grpc::Status& status) { Finish(status); }

    void OnDone() override { delete this; }

    void OnReadDone(bool ok) override {}

    void OnWriteDone(bool ok) override {}
};

class ProcessingImplemantation : public Agent::ShellControllerService::CallbackService {
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

            auto* reactor = new ShellReactor(master_fd, pid);
            reactor->Start();

            return reactor;
        } catch (const std::exception& e) {
            std::cerr << "ShellReactor creation failed: " << e.what() << std::endl;
            return new AbortedReactor(grpc::Status(grpc::StatusCode::INTERNAL, e.what()));
        }
    }
};
}  // namespace Ars::Controller::Shell

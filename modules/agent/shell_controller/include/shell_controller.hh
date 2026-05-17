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

    Agent::ExecuteShellRequest* GetRequest() { return &m_request; }

    void Start() { StartRead(&m_request); }

    void OnReadDone(bool ok) override;
    void OnWriteDone(bool ok) override;

    void OnDone() override {
        m_running = false;
        close(m_master_fd);

        if (m_epoll_cycle.joinable()) {
            m_epoll_cycle.join();
        }

        waitpid(m_pid, nullptr, 0);
        close(m_epoll_fd);

        delete this;
    }

private:
    void DoNextWrite();

    int m_master_fd;
    pid_t m_pid;
    int m_epoll_fd;

    std::thread m_epoll_cycle;
    std::mutex m_queue_mutex;
    std::atomic<bool> m_running{true};
    std::atomic_flag m_writing_in_progress;
    std::condition_variable m_data_notifier;
    std::queue<std::string> m_shell_output_queue;

    OutputCallback m_output_callback;

    Agent::ExecuteShellRequest m_request;
    Agent::ExecuteShellResponse m_response;
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

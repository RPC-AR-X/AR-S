#pragma once

//Standard Includes
#include <fcntl.h>
#include <pty.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

//gRPC Includes
#include <grpcpp/grpcpp.h>
#include "agent.grpc.pb.h"
#include "agent.pb.h"

#include "fd_handler.hh"

namespace Ars::Controller::Shell {
class ShellReactor : public grpc::ServerBidiReactor<Agent::ExecuteShellRequest, Agent::ExecuteShellResponse> {
public:
    using OutputCallback = std::function<void(const std::string&)>;

    explicit ShellReactor(FDHandler&& master_fd_handler, pid_t pid, OutputCallback output_callback = nullptr);

    Agent::ExecuteShellRequest* GetRequest() { return &m_request; }

    void Start() { StartRead(&m_request); }
    void OnReadDone(bool ok) override;
    void OnWriteDone(bool ok) override;

    void OnDone() override {
        m_running = false;
        uint64_t stop_val = 1;
        write(m_signal_fd_handler.get(), &stop_val, sizeof(stop_val));
        if (m_epoll_cycle.joinable()) {
            m_epoll_cycle.join();
        }
        
        delete this;
    }

private:
    void DoNextWrite();

    FDHandler m_master_fd_handler;
    FDHandler m_epoll_fd_handler;
    FDHandler m_signal_fd_handler;
    pid_t m_pid;

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
}  // namespace Ars::Controller::Shell

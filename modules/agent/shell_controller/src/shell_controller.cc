//Standard Includes
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <sys/eventfd.h>
#include <termios.h>
#include <unistd.h>

#include "shell_controller.hh"

namespace Ars::Controller::Shell {
ShellReactor::ShellReactor(FDHandler&& master_fd_handler, pid_t pid, OutputCallback output_callback)
    : m_master_fd_handler(std::move(master_fd_handler)), 
      m_epoll_fd_handler(epoll_create1(0)), 
      m_signal_fd_handler(eventfd(0, EFD_NONBLOCK)),
      m_pid(pid), m_output_callback(std::move(output_callback)) {

    if (m_epoll_fd_handler.get() == -1) {
        throw std::runtime_error("Failed to create epoll instance: " + std::string(strerror(errno)));
    }

    epoll_event main_event{};
    main_event.events = EPOLLIN;
    main_event.data.fd = m_master_fd_handler.get();

    if (epoll_ctl(m_epoll_fd_handler.get(), EPOLL_CTL_ADD, m_master_fd_handler.get(), &main_event) == -1) {
        throw std::runtime_error("Failed to epoll_ctl attached to main_event: " + std::string(strerror(errno)));
    }

    epoll_event signal_fd_event{};
    signal_fd_event.events = EPOLLIN;
    signal_fd_event.data.fd = m_signal_fd_handler.get();

    if (epoll_ctl(m_epoll_fd_handler.get(),EPOLL_CTL_ADD, m_signal_fd_handler.get(), &signal_fd_event) == -1) {
        throw std::runtime_error("Failed to epoll_ctl attached to signal_fd: " + std::string(strerror(errno)));
    }

    m_epoll_cycle = std::thread([&]() {
        epoll_event events[10];
        while (m_running == true) {
            int nfds = epoll_wait(m_epoll_fd_handler.get(), events, 10, -1);
            if (nfds == -1) {
                std::cerr << "epoll_wait error\n";
                break;
            }

            for (int i = 0; i < nfds; ++i) {
                if (events[i].data.fd == m_master_fd_handler.get()) {
                    char buffer[1024];
                    ssize_t count = read(m_master_fd_handler.get(), buffer, sizeof(buffer));
                    if (count == -1) {
                        if (errno != EAGAIN) {
                            std::unique_lock<std::mutex> lock(m_queue_mutex);
                            m_shell_output_queue.push("");
                            m_data_notifier.notify_one();
                        }
                    } else if (count == 0) {
                        std::unique_lock<std::mutex> lock(m_queue_mutex);
                        m_shell_output_queue.push("");
                        m_data_notifier.notify_one();
                        break;
                    } else {
                        std::string shell_data(buffer, count);
                        {
                            std::unique_lock<std::mutex> lock(m_queue_mutex);
                            m_shell_output_queue.push(shell_data);
                        }
                        m_data_notifier.notify_one();
                        DoNextWrite();
                        break;
                    }
                }
                else if (events[i].data.fd == m_signal_fd_handler.get()) {
                    uint64_t dummy;
                    read(m_signal_fd_handler.get(), &dummy, sizeof(dummy));
                    break;
                }
            }
        }
    });
}

void ShellReactor::OnReadDone(bool ok) {
    if (ok) {
        std::string shell_prompt = m_request.command();
        write(m_master_fd_handler.get(), shell_prompt.c_str(), shell_prompt.length());
        StartRead(&m_request);
    } else {
        Finish(grpc::Status::OK);
    }
}

void ShellReactor::OnWriteDone(bool ok) {
    m_writing_in_progress.clear();

    if (ok) {
        DoNextWrite();
    } else {
        OnDone();
        Finish(grpc::Status::CANCELLED);
    }
}

void ShellReactor::DoNextWrite() {
    if (m_writing_in_progress.test_and_set()) {
        return;
    }

    std::unique_lock<std::mutex> lock(m_queue_mutex);
    if (m_shell_output_queue.empty()) {
        m_writing_in_progress.clear();
        return;
    }

    std::string data = m_shell_output_queue.front();
    m_shell_output_queue.pop();
    lock.unlock();

    if (data.empty()) {
        if (!m_output_callback) {
            Finish(grpc::Status::OK);
        }
    } else {
        if (m_output_callback) {
            m_output_callback(data);
        } else {
            m_response.set_output(data);
            StartWrite(&m_response);
        }
    }
}
}  // namespace Ars::Controller::Shell
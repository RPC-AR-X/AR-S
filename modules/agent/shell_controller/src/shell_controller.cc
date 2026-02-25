//Standart Includes
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "shell_controller.hh"

namespace Ars::Controller::Shell {
ShellReactor::ShellReactor(int master_fd, pid_t pid, OutputCallback output_callback)
    : master_fd_(master_fd), pid_(pid), output_callback_(std::move(output_callback)) {
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ == -1) {
        throw std::runtime_error("Failed to create epoll instance: " + std::string(strerror(errno)));
    }

    pid_ = forkpty(&master_fd_, nullptr, nullptr, nullptr);
    if (pid_ == -1) {
        close(epoll_fd_);
        throw std::runtime_error("Failed to forkpty: " + std::string(strerror(errno)));
    }

    if (pid_ == 0) {
        execlp("bash", "bash", nullptr);
        perror("execlp failed");
        exit(1);
    }

    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = master_fd_;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, master_fd_, &event) == -1) {
        close(master_fd_);
        close(epoll_fd_);
        waitpid(pid_, nullptr, 0);
        throw std::runtime_error("Failed to epoll_ctl: " + std::string(strerror(errno)));
    }

    epoll_cycle = std::thread([&]() {
        epoll_event events[10];
        while (running_ == true) {
            int nfds = epoll_wait(epoll_fd_, events, 10, -1);
            if (nfds == -1) {
                std::cerr << "epoll_wait error\n";
                break;
            }

            for (int i = 0; i < nfds; ++i) {
                if (events[i].data.fd == master_fd_) {
                    char buffer[1024];
                    ssize_t count = read(master_fd_, buffer, sizeof(buffer));
                    if (count == -1) {
                        if (errno != EAGAIN) {
                            std::unique_lock<std::mutex> lock(queue_mutex_);
                            shell_output_queue_.push("");
                            data_notifier_.notify_one();
                        }
                    } else if (count == 0) {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        shell_output_queue_.push("");
                        data_notifier_.notify_one();
                        break;
                    } else {
                        std::string shell_data(buffer, count);
                        {
                            std::unique_lock<std::mutex> lock(queue_mutex_);
                            shell_output_queue_.push(shell_data);
                        }
                        data_notifier_.notify_one();
                        DoNextWrite();
                        break;
                    }
                }
            }
        }
    });
}

void ShellReactor::OnReadDone(bool ok) {
    if (ok) {
        std::string shell_prompt = request_.command();
        write(master_fd_, shell_prompt.c_str(), shell_prompt.length());
        StartRead(&request_);
    } else {
        Finish(grpc::Status::OK);
    }
}

void ShellReactor::OnWriteDone(bool ok) {
    writing_in_progress_.clear();

    if (ok) {
        DoNextWrite();
    } else {
        OnDone();
        Finish(grpc::Status::CANCELLED);
    }
}

void ShellReactor::DoNextWrite() {
    if (writing_in_progress_.test_and_set()) {
        return;
    }

    std::unique_lock<std::mutex> lock(queue_mutex_);
    if (shell_output_queue_.empty()) {
        writing_in_progress_.clear();
        return;
    }

    std::string data = shell_output_queue_.front();
    shell_output_queue_.pop();
    lock.unlock();

    if (data.empty()) {
        if (!output_callback_) {
            Finish(grpc::Status::OK);
        }
    } else {
        if (output_callback_) {
            output_callback_(data);
        } else {
            response_.set_output(data);
            StartWrite(&response_);
        }
    }
}
}  // namespace Ars::Controller::Shell
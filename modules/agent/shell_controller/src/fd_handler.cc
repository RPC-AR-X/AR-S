//Standard Includes
#include <unistd.h>

//Libs
#include <spdlog/spdlog.h>

#include "fd_handler.hh"

FDHandler::FDHandler(int fd) : m_fd(fd) {
    spdlog::debug("FDHandler constructed");
}

FDHandler::FDHandler(FDHandler&& other) noexcept {
    spdlog::debug("FDHandler move-constructed");
    this->m_fd = other.m_fd;
    other.m_fd = -1;
}

FDHandler::~FDHandler() {
    spdlog::debug("FDHandler destroyed");

    if (this->m_fd != -1) {
        close(this->m_fd);
    }
}

FDHandler& FDHandler::operator=(FDHandler&& other) {
    spdlog::debug("FDHandler move-assigned");

    if (this != &other) {
        if (this->m_fd != -1) {
            close(this->m_fd);
        }

        this->m_fd = other.m_fd;
        other.m_fd = -1;
    }
    return *this;
}

int FDHandler::get() const {
    return m_fd;
}

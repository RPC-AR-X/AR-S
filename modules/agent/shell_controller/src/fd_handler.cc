//Standard Includes
#include <iostream>
#include <unistd.h>

#include "fd_handler.hh"

FDHandler::FDHandler(int fd) : m_fd(fd) {
    std::cout << "Default ctor" << std::endl;
}

FDHandler::FDHandler(FDHandler&& other) noexcept {
    std::cout << "Moving ctor" << std::endl;
    this->m_fd = other.m_fd;
    other.m_fd = -1;
}

FDHandler::~FDHandler() {
    std::cout << "Default destructor" << std::endl;

    if (this->m_fd != -1) {
        close(this->m_fd);
    }
}

FDHandler& FDHandler::operator=(FDHandler&& other) {
    std::cout << "Moving assignment operator" << std::endl;

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

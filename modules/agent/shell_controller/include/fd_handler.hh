#pragma once

class FDHandler {
public:
    FDHandler(int fd);
    FDHandler(const FDHandler&) = delete;
    FDHandler(FDHandler&& other) noexcept;

    FDHandler& operator=(FDHandler&& other);
    FDHandler& operator=(const FDHandler&) = delete;

    ~FDHandler();

    int get() const;

private:
    int m_fd;
};
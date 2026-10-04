FROM ubuntu:rolling

ENV TZ=Europe/Berlin
ENV DEBIAN_FRONTEND=noninteractive

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && apt-get install -y \
    sudo \
    gnupg \
    curl \
    ca-certificates \
    unzip \
    git \
    build-essential \
    libssl-dev \
    pkg-config \
    clang \
    lld \
    zlib1g-dev \
    qt6-base-dev \
    qt6-declarative-dev \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    libqt6dbus6 \
    libqt6networkauth6-dev \
    nlohmann-json3-dev \
    dbus \
    cmake \
    libsdbus-c++-dev \
    libsdbus-c++-bin \
    libgrpc++-dev \
    libprotobuf-dev \
    libsqlite3-dev \
    libnl-3-dev \
    libnl-genl-3-dev \
    protobuf-compiler-grpc \
    protobuf-compiler \
    libspdlog-dev\
    && rm -rf /var/lib/apt/lists/*

ENV CC=clang
ENV CXX=clang++

RUN useradd -m -s /bin/bash ars && echo "ars:password" | chpasswd
RUN echo "ars ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

USER ars
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y

USER root
RUN ln -s /home/ars/.cargo/bin/rustc /usr/local/bin/rustc && \
    ln -s /home/ars/.cargo/bin/cargo /usr/local/bin/cargo && \
    ln -s /home/ars/.cargo/bin/rustup /usr/local/bin/rustup

USER ars
WORKDIR /home/ars

COPY --chown=ars:ars . .
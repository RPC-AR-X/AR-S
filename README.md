# AR-S
> **Developed by RPC AR-X**

> *"You cannot trust a system you cannot break."*

**AR-S (Advanced Reliable System)** - is an operating system designed for intermediate users. It occupies the pragmatic space between beginner-friendly distributions (like Mint) and hardcore, manual-configuration environments (like Arch or NixOS).

It is built to comfortably handle routine daily tasks, while seamlessly integrating advanced, domain-specific features without the usual configuration friction.

---

## System Stack & Topology

AR-S is logically divided into three operational layers:

* **Base Layer:** **SUSE Linux**. Chosen as the foundation for its robust stability and YaST infrastructure.
* **Orchestration Layer:** The internal communication backbone. It relies on **gRPC** for remote interaction and **D-Bus** for local module coordination.
* **User Space:** A customized **KDE Plasma** environment combined with **Deck** — a standalone Qt-based desktop program that acts as the central control center for configuring AR-S specific modules.
---

## System Components

### Control Interfaces
The external and local access points used to monitor, configure, and route commands to the system daemons.

| Component | Type | Status | Description |
|---|---|---|---|
| **Deck** | GUI Control Center | ![Active](https://img.shields.io/badge/status-active-green) | The standalone Qt-based desktop application for visual management and monitoring of all modules. |
| **Agent** | RPC Gateway | ![Active](https://img.shields.io/badge/status-active-green) | The middleware daemon that receives external gRPC requests and translates them into local IPC commands for the subsystems. |

### Autonomous Daemons
Independent background services, each executing specific domain logic and communicating via strict D-Bus contracts.

| Daemon | Status | Description |
|---|---|---|
| **Interlink** | ![In Progress](https://img.shields.io/badge/status-in%20progress-yellow) | Sovereign communications subsystem natively integrating the encrypted Matrix protocol. |
| **Sonar** | ![Active](https://img.shields.io/badge/status-active-green) | Native OS-level monitoring for CI/CD pipelines and development workflows. |
| **Magfield** | ![Planned](https://img.shields.io/badge/status-planned-lightgrey) | System-level scenario-based automation engine. |

## Getting Started

**Requirement:** AR-S is a Linux-native ecosystem. All development and execution must occur within a Linux environment.

### Dependencies
Reference packages for Debian/Ubuntu environments (package names may vary on other distributions):

```bash
sudo apt update && sudo apt install -y build-essential cmake git clang lld pkg-config libssl-dev zlib1g-dev libsqlite3-dev nlohmann-json3-dev qt6-base-dev qt6-declarative-dev qt6-tools-dev libsdbus-c++-dev libgrpc++-dev libprotobuf-dev libnl-3-dev libnl-genl-3-dev protobuf-compiler-grpc
```

### Building

Configure and build all targets from the project root:

```bash
cmake -B build
cmake --build build
```

> Component-specific build instructions and configuration details are located in the `docs/` directory.

---

## Roadmap

- [x] **Autumn 2025** - MVP of Deck (UI) with Sonar
- [x] **Winter 2025-2026** - JSON view in Deck & Backend Refactoring
- [x] **Spring 2026** - MVP of Interlink

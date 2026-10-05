# AR-S Module: AGENT

* **Role:** Remote Operations Orchestrator (gRPC Gateway)
* **Status:** Active

## Overview

Agent is the external airlock of the **Advanced Reliable System (AR-S)**. It functions as a high-performance gRPC server that exposes internal system capabilities to authorized remote clients (Mobile App).

It acts as a protocol bridge: receiving encrypted gRPC commands from the network and translating them into local system calls or D-Bus messages for specific modules (Sonar, MagField).

## Communication Model

Communication is strictly typed via Protocol Buffers.

## Architecture

The Agent architecture is built around the **Controller Injection** pattern, where the gRPC service delegates actual execution to specialized internal controllers.

### Core Components

1.  **gRPC Service Layer (`agent.proto`)**:
    * Defines the contract for external communication.
    * Handles authentication and request deserialization.

2.  **Internal Controllers**:
    * **Shell Controller** (`Active`): Provides encapsulated access to system shell execution. Unlike a raw SSH session, it executes pre-validated command sets or restricted shell environments.
    * **Sonar Controller** (`Planned`): Acts as a telemetry relay. It proxies remote requests from the mobile app (AR-A) to the local `org.ars.sonar` D-Bus service, allowing the user to view pipeline statuses remotely without direct access to the host's session bus.

* **Port:** Default `50051` (Configurable)
* **Definition:** `protocols/grpc/agent/agent.proto`

## Build & Deployment

Agent depends on the **gRPC** and **Protobuf** ecosystem, managed via CMake.

### Prerequisites
* C++20 Compiler
* `grpc` + `protobuf` (system or submodule)
* `sdbus-c++`

### Compilation

```bash
cmake -B build && cmake --build build --target agent

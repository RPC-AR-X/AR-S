# AR-S Module: SONAR

* **Component Designation:** `org.ars.sonar`
* **Role:** On-demand Pipeline Intelligence
* **Status:** Active

## Overview

Sonar is the reconnaissance unit of the **Advanced Reliable System (AR-S)**. It operates as a background D-Bus service responsible for retrieving CI/CD telemetry.

## Architecture

Sonar implements the **Provider Pattern** to decouple data retrieval from IPC logic.

* **`DbusAdapter`**: Handles D-Bus traffic and commands.
* **`IPipelineProvider`**: Abstract interface for data sources.
* **`GitHubProvider`**: Concrete implementation for GitHub Actions API.

## D-Bus Interface

The module exposes the `org.ars.sonar.Interface` on the session bus.

### Service Details
* **Service Name:** `org.ars.sonar`
* **Object Path:** `/org/ars/sonar`

### Methods

#### `PipelineStatusFetch`
Triggers a synchronized scan across registered providers.
* **Returns:** `s` (JSON String) — Array of pipeline run objects.

#### `UpdateToken`
Authorizes a provider for API access.
* **Input:**
    * `providerName` (string): Target provider ID (e.g., "github").
    * `token` (string): Secure Access Token.
* **Returns:** `b` (boolean) — Success status.

### Signals
* `statusChanging(s statusType)`: Emitted on operational state changes.

## 🛠️ Build & Deployment

### Prerequisites
* C++20 Compiler (GCC/Clang)
* `sdbus-c++`
* `nlohmann-json`
* `cpp-httplib`

### Compilation

```bash
cmake -B build && cmake --build build --target sonar

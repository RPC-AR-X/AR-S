# AR-S Module: DECK

* **Role:** Central Command Interface (HMI)
* **Status:** Active / UX Prototyping

## Overview

Deck serves as the unified Human-Machine Interface (HMI) for the **Advanced Reliable System (AR-S)**. Built on the **Qt 6 (QML)** framework, it provides a hardware-accelerated layer for system interaction.

Unlike traditional desktop environments, Deck does not manage windows; it functions as a **unified control surface**. It aggregates telemetry from D-Bus services (Sonar, Interlink, MagField) and provides direct access to their configuration and operational modes.

## Architecture

Deck implements a strict **Model-View-ViewModel (MVVM)** architecture to maintain separation between the visual presentation and system logic.

* **View (QML)**:
    * Handles purely visual elements and user input.
    * Implements the "Polar Night" design language: strict minimalism, high contrast, and functional negative space.
    * Completely decoupled from business logic.

* **ViewModel (C++)**:
    * Examples: `SonarViewModel`, `AuthManager`.
    * Acts as the translation layer. It subscribes to D-Bus signals from backend daemons and exposes `Q_PROPERTY` fields to the QML engine.
    * Manages transient state (e.g., active input fields) that does not need to persist in the backend.

* **IPC Orchestrator**:
    * Deck functions primarily as a **D-Bus Client**. It does not execute system tasks directly; instead, it dispatches commands (e.g., `UpdateToken`) to the respective specialized agents.

## Build & Deployment

Deck requires the full Qt 6 ecosystem and C++20 support.

### Prerequisites
* **C++20 Compiler** (GCC/Clang)
* **Qt 6.x SDK** (Core, Quick, DBus, Qml modules)
* `sdbus-c++`

### Compilation

```bash
cmake -B build && cmake --build build --target deck

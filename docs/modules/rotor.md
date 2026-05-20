# AR-S Module: ROTOR

* **Role:** Dual-State Tor Environment
* **Status:** Active / Prototyping

## Overview

Rotor is the privacy orchestration layer of the **Advanced Reliable System (AR-S)**. It extends the standard Tor Browser with an optional convenience-oriented environment while preserving the ability to instantly return to a clean isolated Tor session.

The module is designed around reversible state transitions, allowing advanced integrations and persistent workflows without permanently modifying the trusted Tor runtime.

## Architecture
> ⚠ Architecture is currently under active development and may change significantly between revisions.

The module is divided into several isolated components with strict responsibility boundaries:

* **ReD (Relay Daemon)**:
    * Receives requests from the browser extension and coordinates privileged operations.
    * Collects requested data from Vault only after successful verification approval from the Verification Agent.
    * Translates raw byte streams into structured JSON messages and sends responses back to the browser extension.
    * Contains the internal **Access Controller** layer responsible for validating and filtering privileged requests before execution.

* **Verification Agent**:
    * User confirmation layer for sensitive operations.
    * Displays secure approval dialogs before protected actions are executed.

* **Vault**:
    * Dedicated encrypted credential storage daemon.
    * The only component allowed to directly access and manage sensitive user secrets such as passwords and authentication data.

* **Profile Swapper**:
    * Manages transitions between browser environments.
    * Separates lightweight session serialization from profile directory manipulation.

* **State Switcher**:
    * Tray-based orchestration daemon responsible for coordinating mode transitions.
    * Controls the browser restart lifecycle and transition progress states.

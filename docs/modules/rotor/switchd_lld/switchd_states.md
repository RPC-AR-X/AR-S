# Switchd
**Status:** Active / Prototyping

## Architecture
> ⚠ Architecture is currently under active development and may change significantly between revisions.

## State Matrix

| State             | Description                                                                              | Event                                 | Next State        |
|-------------------|------------------------------------------------------------------------------------------|---------------------------------------|-------------------|
| `INIT`            | Start of the profile switching pipeline.                                                 | Profile switching has been initiated. | `HALTING_PROCESS` |
| `HALTING_PROCESS` | Switchd performs a graceful Tor Browser shutdown and waits for the process to terminate. | Tor Browser terminated successfully.  | `DIR_MOVING`      |
|                   |                                                                                          | Shutdown timeout exceeded.            | `TIMED_OUT`       |
|                   |                                                                                          | Profile switching was cancelled.      | `ABORTED`         |
| `DIR_MOVING`      | Profile directory moves to another place                                                 | Successful moving directory           | `FINAL`           |
|                   |                                                                                          | Moving directory was cancelled        | `ABORTED`         |
|                   |                                                                                          | Moving directory failed               | `ERROR`           |
| `TIMED_OUT`       | The allowed operation waiting time has been exceeded.                                    | Timeout occurred.                     | Terminal          |
| `ABORTED`         | Current profile switching operation has been aborted.                                    | Operation was cancelled.              | Terminal          |
| `FINAL`           | Profile switching pipeline has completed successfully.                                   | All required operations completed.    | Terminal          |

### Actions / Events That Are Not Switchd States

- `DBUS_SWAPD_REQUEST` — D-Bus call to Swapd.
- `NOT_RECEIVED` — result of a failed IPC call.
- Serialization `ABORTED` as an internal Swapd event is represented by the general `ABORTED` state in Switchd.
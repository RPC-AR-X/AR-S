# Switchd
**Status:** Active / Prototyping

## Architecture
> ⚠ Architecture is currently under active development and may change significantly between revisions.

## State Matrix

| State             | Description                                                                                     | Event                                      | Next State |
|-------------------|-------------------------------------------------------------------------------------------------|--------------------------------------------|------------|
| `INIT`            | Start of the profile switching pipeline.                                                        | Profile switching has been initiated.      | `HALTING_PROCESS` |
| `HALTING_PROCESS` | Switchd performs a graceful Tor Browser shutdown and waits for the process to terminate.        | Tor Browser terminated successfully.       | Next pipeline stage |
| `HALTING_PROCESS` | Switchd performs a graceful Tor Browser shutdown and waits for the process to terminate.        | Shutdown timeout exceeded.                 | `TIMED_OUT` |
| `HALTING_PROCESS` | Switchd performs a graceful Tor Browser shutdown and waits for the process to terminate.        | Profile switching was cancelled.           | `ABORTED` |
| `TIMED_OUT`       | The allowed operation waiting time has been exceeded.                                           | Timeout occurred.                          | Terminal |
| `ABORTED`         | Current profile switching operation has been aborted.                                           | Operation was cancelled.                   | Terminal |
| `IN_PROGRESS`     | Temporary placeholder for the Profile Swapping stage. Details will be defined during Swapd LLD. | Profile directory switching completed.     | `FINAL` |
| `IN_PROGRESS`     | Temporary placeholder for the Profile Swapping stage. Details will be defined during Swapd LLD. | Other transition condition.                | Not yet defined |
| `FINAL`           | Profile switching pipeline has completed successfully.                                          | All required operations completed.         | Terminal |

### Actions / Events That Are Not Switchd States

- `DBUS_SWAPD_REQUEST` — D-Bus call to Swapd.
- `NOT_RECEIVED` — result of a failed IPC call.
- `INIT_SER` — internal Swapd state.
- `DONE_SER` — internal Swapd state.
- `ERROR_SER` — internal Swapd state.
- Serialization `ABORTED` as an internal Swapd event is represented by the general `ABORTED` state in Switchd.

### Undefined Part of the Draft

There is currently no approved Switchd state between `HALTING_PROCESS` and `IN_PROGRESS` representing the wait for serialization performed by Swapd. No new state is intentionally introduced until further LLD work is completed.
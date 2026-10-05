# Agent

**Status:** Active / Prototyping

## Architecture
> ⚠ Architecture is currently under active development and may change significantly between revisions.

The code has no separate state enumeration or transition table. The names below are analytical labels for existing flags and callbacks, not new entities. The lifecycle, write queue, and PTY loop are partially independent; they have not been combined into an invented single state machine.

## Reactor lifecycle

| Current state | Event / condition | Actual action | Next state / limitation |
|---|---|---|---|
| Creation | `forkpty < 0` | Create `AbortedReactor`, `Finish(INTERNAL)` | Waiting for `OnDone` |
| Creation | `std::exception` thrown | Logging; `AbortedReactor`, `Finish(INTERNAL)` | Waiting for `OnDone` |
| Creation | Successful `ShellReactor` creation, `Start()` | PTY thread started; `StartRead` | Request read active |
| Request read active | `OnReadDone(true)` | Write `command` to PTY; `StartRead` | Request read active |
| Request read active | `OnReadDone(false)` | `Finish(OK)` | Shutdown requested; PTY has not stopped yet |
| Live `ShellReactor` | `OnDone()` from gRPC | `m_running=false`; write to `eventfd`; `join`; `delete this` | Object deleted |
| Write active | `OnWriteDone(false)` | Direct `OnDone()` → deletion; then `Finish(CANCELLED)` | **Undefined behavior**, no valid transition |
| `AbortedReactor` after `Finish` | `OnDone()` | `delete this` | Object deleted |

## Queue dispatch

| Current state | Event / condition | Actual action | Next state |
|---|---|---|---|
| Idle: flag clear | `DoNextWrite`; queue empty | `test_and_set`; check under mutex; `clear` | Idle |
| Busy: flag set | `DoNextWrite` | Return without processing | Busy |
| Idle | Non-empty string in queue; no callback | Remove string; populate `m_response`; `StartWrite` | Write active, flag set |
| Write active | `OnWriteDone(true)` | `clear`; `DoNextWrite` | Idle or next write |
| Idle | Empty string in queue; no callback | `Finish(OK)` | Shutdown requested; flag remains set |
| Idle | Non-empty string; callback set | Invoke callback | Flag remains set; continuation not implemented |
| Idle | Empty string; callback set | String removed; `Finish` not called | Flag remains set |

## PTY loop

| State | Event | Action | Result |
|---|---|---|---|
| `m_running=true` | `epoll_wait` returned `-1` | Logging, exit `while` | Thread has ended, but flag is unchanged and `Finish` has not been called |
| `m_running=true` | `read > 0` | Add bytes; notify; `DoNextWrite` | Next iteration |
| `m_running=true` | `read == -1`, `EAGAIN` | Add no data | Next event / iteration |
| `m_running=true` | `read == -1`, other error | Add empty string; notify | No direct call to `DoNextWrite` |
| `m_running=true` | `read == 0` | Add empty string; notify; exit inner `for` | Outer `while` continues |
| During `OnDone` | `m_running=false`, `eventfd` event | Read counter; exit `for`; check `while` | Thread terminates; `join` allows deletion |

**Modeling limitation:** concurrent gRPC shutdown and sending from the PTY thread have no shared state barrier. This table therefore does not prove the correctness of all concurrent transitions; it accurately lists the implementation branches.

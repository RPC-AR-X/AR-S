# Sonar

**Status:** Active / Prototyping

## Architecture
> ⚠ Architecture is currently under active development and may change significantly between revisions.

**Sonar has no explicit finite state machine (FSM).** The table below describes only the state that follows directly from the `m_current_token` field and the `empty()` check. A nonempty token does not imply valid authorization. State names are introduced for descriptive purposes and do not exist in the code.

| Current State | Event / Condition | Action | Next State |
|---|---|---|---|
| Creation | `GitHubProvider` constructor | The token string is initially empty | Token empty |
| Any | `UpdateToken`, name does not equal `Github` | Return false; `SetToken` is not called | Unchanged |
| Any | `UpdateToken("Github", nonempty string)` | Assign the token; return true | Token nonempty |
| Any | `UpdateToken("Github", "")` | Assign an empty token; return true | Token empty |
| Token empty | `FetchStatusAsJson` | Return an empty string without HTTP | Token empty |
| Token nonempty | `FetchStatusAsJson`, success | HTTP, filtering, file write, return JSON | Token nonempty |
| Token nonempty | HTTP, network, or JSON error | Log the error; return an empty string | Token nonempty; no automatic clearing |

An in-progress request, an API error, and process termination are not stored as separate provider states. They are therefore not represented as invented transitions in a domain state machine. The process startup/shutdown sequence is described in `sonar-lld.md`.

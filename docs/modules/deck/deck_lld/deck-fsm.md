# Deck

**Status:** Active / Prototyping

## Architecture
> ⚠ Architecture is currently under active development and may change significantly between revisions.

## Obtaining the Code and Polling for the Token

| State | Event / Condition | Implementation Action | Next State / Limit |
|---|---|---|---|
| Before Starting | `startAuth`, empty client ID | `tokenErrorReceived("Client ID missing")` | Request is not started |
| Before Starting | `startAuth`, ID is set | `grant`, POST for the device code | Code Request in Progress |
| Code Request in Progress | Network error | `errorOccured`, delete reply | No new timer is started; AuthManager does not forward the signal to QML |
| Code Request in Progress | No network error | Read JSON fields, `authorizeWithUserCode`, start timer `(interval+1)*1000` | Polling; field completeness is not checked separately |
| Polling | Timer fires | `pollToken`, POST for the token | Timer is active, awaiting response |
| Polling | Response contains `access_token` | Store the token, stop the timer, `granted` | Token Received |
| Polling | `authorization_pending` | No additional action | Polling continues |
| Polling | `slow_down` | `setInterval((newInterval+1)*1000)` | Polling with an adjusted interval |
| Polling | `expired_token` | Stop the timer, `errorOccured` | Polling stopped |
| Polling | Other `error`, missing token/error, or network error | No terminating transition; reply is deleted | Polling continues |
| Any | Repeated `grant()` | New POST without a busy check or an explicit reset of the previous session | Overlap is possible; the code does not define a single next state |

**Limit:** the timer is not stopped during a network request. Multiple `pollToken()` calls may be pending simultaneously. Stopping the timer does not cancel requests already sent. The table does not assume that responses are mutually exclusive.

## Token Transfer and Visible QML State

| State | Event | Action | Next State |
|---|---|---|---|
| `isGithubAuthenticated=false` | `deviceAuthReady` | Fill in URL/code, open AuthPopup | Flag remains false |
| Token Received | `granted` → `onAuthFinished` | Log the token, synchronous `UpdateToken("Github", token)` | Awaiting D-Bus response |
| Awaiting D-Bus | Success and `true` | Log success; `tokenReceived` | QML closes the window, sets true |
| Awaiting D-Bus | Error or `false` | Log error; **also** `tokenReceived` | QML closes the window, sets true |
| AuthPopup Open | Escape or click outside | Window closes | OAuth is not canceled; flag is unchanged |
| `isGithubAuthenticated=true` | Subsequent operation | List and Refresh are visible | The code has no transition back to false |

## Run Model

This is a table of request processing outcomes, not a separate state machine in the code.

| Event | Data Change | Signal / Completion |
|---|---|---|
| `fetchPipelineData` | Previous data remains; a watcher is created | Asynchronous request |
| Successful response, JSON array | Clear and repopulate the model | `pipelineDataReceived`; defer watcher deletion |
| Successful response, not an array | Model is already cleared | Early return without an error signal or `deleteLater` for the watcher |
| D-Bus error | Previous model remains | `pipelineDataErrorReceived`; defer watcher deletion |

There are no separate “loading” or “error” states, nor any mechanism to block repeated Refresh requests. They have not been added as though they were implemented.

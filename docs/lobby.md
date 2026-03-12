# Interactive Lobby & Matchmaking System

The lobby system is more than just a static menu—it is a fully synchronized, animated pre-game room built on top of the custom State Machine and TCP networking stack. It handles game discovery, configuration, and the critical transition into the gameplay state.

Here is how the system is structured:

### 1. Dynamic Server Browser (`JoinLobbyState`)
Instead of forcing players to type IP addresses, the game features a dynamic server browser.
* When a player clicks "Join Game", the state starts listening for UDP broadcasts.
* The UI dynamically generates a list of available lobbies, displaying the Room Name, Host Name, and IP address.
* Players can refresh the list or simply click "Join" on a specific entry, which instantly establishes the TCP connection.

### 2. The Waiting Room (`LobbyWaitingState`)
When a player creates a game, they enter a transitional waiting state.
* The game binds to a TCP port and begins broadcasting its existence over UDP.
* The UI displays an animated "Waiting for player..." text while a background thread safely waits for an incoming TCP connection.
* Once a connection is detected, the state automatically pushes the main `LobbyState` onto the stack.

### 3. Synchronized Room (`LobbyState`)
Once both players are in the lobby, the `NetworkLobbyManager` takes over to keep everything perfectly synced via TCP.
* **Host Privileges:** Only the Host can change game settings like the *Number of Rounds* or *Time Limit*. When the host clicks the UI buttons, the new settings are instantly sent to the client and logged in the chat.
* **Integrated Chat:** A custom text field system allows players to chat in real-time. The chat automatically handles text wrapping for long messages.
* **Ready System:** Both players must click "Ready" / "Start Game". The game waits for `__HOST_READY__` and `__CLIENT_READY__` packets before initiating the match countdown.

### 4. Seamless Socket Handoff
This is the most critical part of the lobby architecture. When the game starts, the TCP connection cannot be dropped, but the Lobby UI must be destroyed.
* Instead of closing the connection, the lobby calls `ReleaseSocket()`. This stops the lobby's network thread but keeps the underlying UNIX socket open.
* The raw socket integer is then injected directly into the constructor of the new `GameplayState`.
* This enables a perfectly seamless transition from the UI room into the arena without ever losing the connection to the other player.

### 5. Animated UI
To make the experience feel polished, every state includes smooth slide-in and slide-out animations (`ENTERING` / `EXITING` states). UI elements smoothly move across the screen when transitioning between menus, ensuring the game feels modern and responsive.
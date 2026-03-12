# Networking Architecture: TCP/UDP Hybrid

The networking system is built from scratch using raw UNIX sockets (`<sys/socket.h>`). It uses a hybrid approach to solve a classic multiplayer problem: **TCP is too slow for fast-paced shooting, but UDP is too unreliable for setting up a game lobby.**

Here is how the custom network stack handles a complete game lifecycle:

### 1. Auto-Discovery (No IP Typing)
Players shouldn't have to manually type IP addresses to play on a local network.
* **The Host** uses a background thread to constantly broadcast a UDP packet on port `8888` saying *"I have an open lobby"*.
* **The Guest** listens for these broadcasts. Discovered lobbies are saved in a map. If a host disconnects and stops broadcasting for 5 seconds, the lobby is automatically removed from the list.

### 2. The Lobby (TCP)
Once a player clicks on a lobby, the game establishes a reliable TCP connection. 
* TCP is used here because we cannot afford to lose packets containing critical data like player nicknames or "Ready" statuses.
* The server uses a non-blocking `select()` loop to safely read incoming messages separated by a newline (`\n`), preventing packet fragmentation.

### 3. The "Magic" Switch (TCP -> UDP)
When both players are ready, the game must seamlessly switch from TCP to UDP for combat.
* To prevent desync (e.g., one player switching to UDP while the other is still loading), the engine performs a strict **TCP Handshake**.
* Both machines exchange "magic numbers" (`0x11223344` and `0x55667788`) over TCP to confirm they are perfectly synced. 
* Once confirmed, they dynamically bind to available UDP ports and securely exchange those port numbers over the TCP line.

### 4. Real-Time Combat (Custom String Protocol)
During the match, all data goes through UDP to guarantee zero-latency combat. Instead of sending complex byte structures, the game uses a fast, lightweight, custom string protocol. 
Examples of packets:
* `"pos;120.5;340.0"` -> Updates enemy coordinates.
* `"gun_angle;45.5"` -> Syncs weapon rotation.
* `"hit_player"` -> Sent instantly when a local collision is registered.

This text-based approach made debugging network traffic incredibly fast and efficient.

### 5. Multithreading for a Stable 60 FPS
Network functions (like waiting for packets) can freeze the game and cause stuttering. To prevent this entirely:
* All network managers run on dedicated background `std::thread`s.
* Incoming messages are pushed into a `std::queue<std::string>`.
* This queue is strictly protected by a `std::mutex` and a `std::condition_variable`. 
* The main game loop simply pops messages from this queue once per frame, keeping the gameplay locked at a perfectly smooth 60 FPS without any race conditions.
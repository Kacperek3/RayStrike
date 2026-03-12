<h1 align="center">RayStrike: C++ Multiplayer Shooter</h1>

<p align="center">
  Multiplayer top-down shooter built entirely from scratch in <strong>C++17</strong> and sfml.
  <strong>7,500+ lines </strong> of pure code across <strong> 50+ source files </strong>. Zero external engines.
</p>


<p align="center">
  <img src="https://img.shields.io/badge/C++-17-blue.svg?style=for-the-badge&logo=c%2B%2B" alt="C++">
  <img src="https://img.shields.io/badge/SFML-2.6-green.svg?style=for-the-badge" alt="SFML">
  <img src="https://img.shields.io/badge/CMake-Build-red.svg?style=for-the-badge&logo=cmake" alt="CMake">
  <img src="https://img.shields.io/badge/Networking-UDP%2FTCP-purple.svg?style=for-the-badge" alt="Networking">
  <img src="https://img.shields.io/badge/Lines_of_Code-7500+-black.svg?style=for-the-badge" alt="7500+ LOC">
    <img src="https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge" alt="License">


</p>

<br>


---

## Why RayStrike?

> [!IMPORTANT]
> **Zero accounts. Zero 50GB updates. Zero matchmaking queues.**
>
> RayStrike is a pure, plug-and-play 1v1 arena shooter built for immediate action. Launch the game, instantly find your friend's LAN lobby, and settle your scores. Perfect for a 15-minute break or a quick LAN party showdown.

---
## Features

* **Custom State Machine:** Memory-safe scene transitions. [Read more](docs/engine.md)
* **Custom TCP/UDP Communication Protocol:** Built from scratch using raw sockets. TCP ensures reliable lobby connections, while a custom UDP protocol guarantees zero-latency combat sync. [Read more](docs/networking.md)
* **Multithreaded Architecture:** Background network I/O ensures an unblocked, locked-60-FPS game loop. [Read more](docs/multithreading.md)
* **Lobby System:** Host/join rooms, nickname synchronization, and ready-up states. [Read more](docs/lobby.md)
* **1v1 Arena Combat:** Fast-paced top-down shooting. Features custom hit detection, smooth movement, and a clean UI. [Read more](docs/game.md)

---

## Multiplayer in Action

<h3 align="center">1. LAN Auto-Discovery & Match Creation</h3>
<table align="center">
  <tr>
    <th align="center">Host POV (Creating Match)</th>
    <th align="center">Guest POV (Joining Match)</th>
  </tr>
  <tr>
    <td width="50%">
      <img src="assets/gifs/LobbyCreation.gif" alt="Host opening a UDP broadcast room" width="100%"/>
    </td>
    <td width="50%">
      <img src="assets/gifs/LobbyJoin.gif" alt="Guest finding room via server browser" width="100%"/>
    </td>
  </tr>
  <tr>
    <td colspan="2" align="center">
      <i>The Host broadcasts the room on the local network while the Guest's browser detects it instantly. Zero manual IP typing.</i>
    </td>
  </tr>
</table>

<br>

<h3 align="center">2. TCP Synchronized Lobby</h3>
<table align="center">
  <tr>
    <th align="center">Host POV (Configuring Match)</th>
    <th align="center">Guest POV (Live Updates)</th>
  </tr>
  <tr>
    <td width="50%">
      <img src="assets/gifs/LobbyHost.gif" alt="Host changing game settings" width="100%"/>
    </td>
    <td width="50%">
      <img src="assets/gifs/LobbyGuest1.gif" alt="Guest receiving live settings update" width="100%"/>
    </td>
  </tr>
  <tr>
    <td colspan="2" align="center">
      <i>Game configurations, ready states, and chat messages are synchronized over TCP before the match begins.</i>
    </td>
  </tr>
</table>

<br>

<h3 align="center">3. Real-Time Arena Combat</h3>
<p align="center">
  <img src="assets/gifs/Gameplay1.png" alt="Fast-paced 1v1 combat showcase" width="85%"/>
</p>
<p align="center">
  <i>Combat runs on custom UDP sockets to ensure real-time shooting and reliable hit registration.</i>
</p>

---
## ️ Installation & Building
This project uses **CMake** for building. I have provided a simple helper script to automate the process for you.

### Prerequisites
* **Linux:** `g++`, `cmake`, `libsfml-dev`
* **Windows:** Visual Studio (with C++) or MinGW, and CMake.

###  The "One-Click" Run (Recommended)

1.  Clone the repository:
    ```bash
    git clone https://github.com/Kacperek3/RayStrike.git
    cd Raystrike
    ```

2.  Run the build script:
    ```bash
    # Make the script executable (Linux/Mac only)
    chmod +x build_and_run.sh

    # Build and Run in Release mode 
    ./build_and_run.sh Release
    ```

**That's it!** The script will automatically configure CMake, compile the source code, and launch the game.

---

## Controls

| Key / Action | Description |
| :--- | :--- |
| **W, A, S, D** | Move your character |
| **Mouse** | Aim your weapon |
| **Left Click** | Fire |
| **R** | Request rematch (when the round is over) |

---
## Development Roadmap

**Phase 1: Core Foundation (Completed)**
- [x] Custom C++ Engine & State Machine
- [x] Hybrid TCP/UDP Networking Protocol
- [x] Asset & Audio Management System

**Phase 2: Gameplay Expansion (Up Next)**
- [ ] New Weapons (Different types and fire rates)
- [ ] New Maps and Obstacles
- [ ] Bullet Ricochets and Power-ups


---

## Contributing

RayStrike is an open-source passion project. Whether you want to fix a bug, optimize the network code, or add a new weapon from the roadmap, your help is highly welcome.

If you want to contribute, please follow the standard GitHub workflow:
1. Fork the repository.
2. Create a new branch for your feature
3. Commit your changes
4. Push to the branch 
5. Open a Pull Request.

## Credits & Acknowledgements

This game was built from scratch as a collaborative effort. A massive thank you to my co-developer who helped bring this project to life:

* **Kacperek3** – Project Lead: Custom networking stack, combat mechanics, and UI implementation.
* **Pete939** – Core Contributor: Engineered base game engine.

---

##  License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
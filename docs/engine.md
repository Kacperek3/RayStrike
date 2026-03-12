# Core Engine Architecture

RayStrike is powered by a custom-built, lightweight C++ engine designed specifically for 2D SFML games. It avoids the overhead of general-purpose engines, focusing entirely on performance, strict memory management, and a clean game loop.

## Architecture Overview

The engine is built around a centralized `GameData` structure. This struct holds the `sf::RenderWindow` and all core system managers. It is passed to every game state via a `std::shared_ptr`, ensuring safe and global access without relying on anti-patterns like singletons.

### 1. Stack-Based State Machine
The flow of the game is controlled by the `GameStateManager`.
* **Stack Architecture:** Game scenes (Menu, Lobby, Gameplay) are pushed and popped from a standard stack. 
* **State Preservation:** The stack allows states to be paused and resumed. For example, a settings overlay can be pushed on top of the gameplay state without destroying the ongoing match.
* **Memory Safety:** States are wrapped in `std::unique_ptr` (`StateRef`). When a state is removed or replaced, its memory is automatically and cleanly deallocated.

### 2. The Game Loop
The main loop enforces a strict, predictable pipeline. Every active state must implement pure virtual methods for input handling, logic updates, and rendering:

```cpp
void Game::Run() {
    while (this->_data->window.isOpen()) {
        this->_data->stateManager.ProcessStateChanges();
        this->_data->stateManager.GetActiveState()->HandleInput();
        this->_data->stateManager.GetActiveState()->Update();
        this->_data->stateManager.GetActiveState()->Draw();
    }
}
```


### 3. Resource Management
To ensure the game runs flawlessly without memory leaks or disk I/O bottlenecks, assets are managed through dedicated classes:
* **AssetManager:** Caches `sf::Texture` and `sf::Font` instances in memory using `std::map`. It actively prevents duplicate loading of the same assets from the disk.
* **SoundManager:** Handles the loading and mapping of `sf::SoundBuffer` objects, generating playable `sf::Sound` instances on demand.
* **InputManager:** Beyond standard AABB bounding box checks, it features **pixel-perfect collision detection** (`IsSpriteClickedAccurate`). It maps mouse coordinates to texture space and reads the alpha channel to ensure clicks register only on visible pixels, not transparent bounds.

### 4. Core Engine File Structure
The engine's architecture is cleanly separated into headers and source files, keeping declarations and implementations strictly modular. 

```text
src/
├── Game.h                 # Main application loop and GameData struct
├── Game.cpp               # Window initialization and StateManager execution
├── State.h                # Abstract base class (interface) for all game scenes
├── GameStateManager.h     # Stack-based state controller
├── GameStateManager.cpp
├── AssetManager.h         # Texture and font caching system
├── AssetManager.cpp
├── InputManager.h         # Input handling and pixel-perfect collisions
├── InputManager.cpp
├── SoundManager.h         # Audio buffer and sound instantiation
└── SoundManager.cpp
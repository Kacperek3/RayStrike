# Multithreading & Performance

Multiplayer games often stutter when waiting for network data. RayStrike solves this by running all networking in the background, keeping the main game loop locked at a perfectly smooth 60 FPS. 

Here is how the threading is structured to prevent freezing and crashing:

### 1. Separation of Tasks
The engine strictly divides the workload:
* **The Main Thread:** Only cares about reading keyboard input, updating physics, and drawing sprites to the screen. It never waits for the network.
* **Background Threads:** Every network manager creates its own `std::thread` just for listening to sockets. These threads can sit and wait for packets all day without freezing the game window.

### 2. The Message Queue
Background threads and the main thread need a safe way to talk to each other without causing crashes.
* When a network thread receives a packet, it pushes the string data into a `std::queue<std::string>`.
* To do this safely, it uses a `std::condition_variable` and a `std::mutex` to lock the queue for a microsecond while the data is inserted.
* The Main Thread simply checks this queue once per frame (`PopMessage()`). If there's a new message, it reads it. If the queue is empty, it just moves on and renders the next frame. No waiting, no frame drops.

### 3. Preventing Data Races (Mutexes)
If the network thread updates the enemy's health at the exact same millisecond the main thread is trying to draw the health bar, the game will crash (Data Race).
* To prevent this, active game states use a dedicated `std::mutex` (like `_gameStateMutex`).
* Any time a thread wants to read or modify player stats or bullet positions, it must use a `std::lock_guard<std::mutex>`. This guarantees that only one thread can touch the critical data at any given time, making the game mathematically safe from read/write tearing.

### 4. Safe Shutdown
When a player leaves a lobby or closes the game, background threads must be closed properly. 
* The engine uses `std::atomic<bool>` flags (e.g., `_running = false;`) to signal the background loops to stop.
* Inside the destructors, the game explicitly calls `.join()` on the threads. This forces the engine to wait for the thread to safely finish its current loop before deleting the object, entirely preventing memory leaks and dangling threads.
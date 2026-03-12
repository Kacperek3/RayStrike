# Arena Combat & Gameplay Mechanics

Once the network seamlessly transitions from the TCP lobby to the UDP arena, the `GameplayState` takes over. The combat loop is designed to be fast, responsive, and mathematically precise, relying purely on C++ and SFML without any heavy physics engines.

Here is how the core combat mechanics are implemented:

### 1. Top-Down Movement & Aiming
The control scheme is built for precision in a 2D space.
* **Movement:** The W, A, S, and D keys apply immediate velocity vectors to the player's core hitbox. The engine also clamps the coordinates to ensure the player cannot walk off the screen.
* **Aiming:** The game constantly tracks the mouse coordinates. It uses the `atan2` mathematical function to calculate the exact angle between the player's center and the cursor. This angle is used to instantly rotate the gun sprite and set the trajectory for fired bullets.

### 2. Precise AABB Collisions
To keep combat fair, hit detection must be flawless and lightweight.
* **Bounding Boxes:** Every interactive object (players, bullets, walls) has a dedicated hitbox. The game uses Axis-Aligned Bounding Box (AABB) logic via SFML's `getGlobalBounds().intersects()` function to check for overlaps every single frame.
* **Environment Blocking:** If a player's movement vector causes their hitbox to intersect with a static neon wall, the movement is instantly reverted. Bullets that hit walls are immediately erased from the active vector to free up memory.
* **Hit Registration:** When a bullet's hitbox intersects with an enemy's hitbox, the game deducts health points and destroys the bullet.

### 3. Dynamic User Interface
The UI is rendered dynamically on top of the game world, updating strictly based on the current game state.
* **Health Bars:** Instead of using pre-rendered images for health, the game draws dynamic rectangles. The width of the inner health bar is mathematically calculated on the fly as a percentage of the player's current health (`width * (health / 100.f)`). Local players see a green bar, while the enemy bar is drawn red.
* **Custom Crosshair:** The standard operating system mouse cursor is hidden. In its place, the game draws a custom, geometric crosshair (a center dot with surrounding lines) precisely at the mouse coordinates to help with aiming.
* **Live Scoreboard:** The top of the screen displays the current score. This text is updated immediately when a round concludes, keeping both players informed.

### 4. Game Loop & Round Reset
The game handles win conditions without breaking the network connection.
* **Win State:** The engine continuously monitors the health of both players. The moment a player's health reaches zero, the combat logic halts, and the "Round Over" sequence triggers.
* **Instant Restart:** Instead of kicking players back to the lobby, both the Host and Guest can press 'R' to request a rematch. Once confirmed, the game clears all active bullets, resets health to 100, and restores starting positions, allowing the next round to begin instantly.
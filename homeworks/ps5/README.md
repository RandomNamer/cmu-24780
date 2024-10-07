![image-20241002021355026](/Users/zzy/Library/Application Support/typora-user-images/image-20241002021355026.png)

# A COD Parody

More specifically, a simple shooter game with...

1. **Dynamic Targets**: Spawn multiple enemies with different distances, moving along the x-axis and challenge your aim as they dart across your screen.
2. **Mouse Control**: Look around freely with your mouse to find more enemies.
3. **Familiar Shooting Mechanics**: Use well-known controls like the left mouse button to fire, the right to aim down sights (ADS), R to reload, and 1 to switch weapons. 
4. **FOV control and ADS Experience:** Smooth ADS animation based on FOV changes, as the majority of FPS games do. Change mouse sensitvity based on FOV in real-time so you can do quick-scoping snipes.
5. **Build Your Weapon**: Act like a gunsmith by customizing core weapon properties—ADS time, fire rate, reload speed, scope magnification, and damage—to recreate your favorite COD weapons. Preloaded with 2 iconic weapons with specs from *Modern Warfare III (2023)*, by courtesy of [sym.gg](https://sym.gg). 
6. **Authentic Sound Effects**: Enjoy the iconic hitmarker sound from the original *Modern Warfare* and finishing sounds from *Modern Warfare II (2022)* for that extra touch of nostalgia and satisfaction.

## Before getting started

1. Change the `RES_DIR` definition to your local directory where resource file resides.
2. If you like, feel free to tweak the `GameConfig` with target count, target move speed and mouse sensitivity.
3. Make sure to use a real mouse, not a trackpad for the full ADS experience!

## Implementation

1. Using GL primitives like `GL_LINE_LOOP`, `GL_QUADS`, `GL_TRIANGLE_FAN` and more to draw elements.
2. Event driven. Handle game inputs and dispatch in-game events to control rendering and sound playback. Used a priority queue with custom comparator to handle events like display hitmarker, display scope, ADS, update view port and open fire.
3. FOV control: compute current view port in world canvas for each frame using FOV and aim center, then render targets with transformed coordinates  accordingly.
4. Implemented as a game class and clearly structured. The game class holds all states and expose only a `doFrame` method for the main loop to feed events like mouse movement, keyboard stroke and vsync into the game.
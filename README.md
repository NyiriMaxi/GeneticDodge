# GeneticDodge

GeneticDodge is a real-time C++ and SFML simulation where a population of AI-controlled players learns to avoid falling obstacles through evolution. It uses no labeled training data: each generation is scored by survival time, and the strongest brains are selected to produce the next generation.

## How it works

- The simulation runs a population of 50 players and spawns falling obstacles.
- Each brain receives the player's normalized horizontal position and information about up to 10 obstacles: their horizontal offset and signed time-to-player-height. Unused obstacle slots receive a sentinel value.
- A small neural network scores three actions: move left, stay, or move right.
- Survival time determines fitness. The genetic algorithm keeps elite brains, selects parents, crosses their weights, and applies random mutations.
- A short-horizon safety check predicts likely collisions and can override the brain's chosen action when that action is unsafe. This safety check is hand-coded; it is not part of the learned policy.
- A timer displays elapsed time for the current generation.

The current settings and implementation details are defined in the source files and can be tuned there.

## Requirements

- MSVC v143 toolset and a Windows SDK
- C++17
- SFML_VS2019 NuGet package, version 1.0.0, restored through NuGet

## Build and run

1. Open the solution in Visual Studio.
2. Restore NuGet packages if Visual Studio does not do so automatically.
3. Select a configuration and platform, such as Debug | x64.
4. Build and run the GeneticDodge project.
5. Set the debugger working directory to the project directory so relative asset paths resolve correctly.

The program currently loads the player sprite from ../Images/PlayerSprite.png and the font from Sakire.ttf. The solution file and the Images directory are currently outside this repository's Git root (see Repository layout below); a fresh clone therefore needs those files placed at the expected relative paths, or the solution and asset paths should be moved/updated before building.

## Controls and display

The window shows the active population, falling obstacles, generation number, and elapsed time for the current generation. Players are controlled by their brains during evolution; there are no gameplay controls required.

## Source files

| File | Responsibility |
| --- | --- |
| main.cpp | Creates the window and runs the simulation/update loop. |
| Player.h / Player.cpp | Player state, movement, sprite rendering, and collision behavior. |
| Obstacles.h / Obstacles.cpp | Obstacle state, movement, spawning, and rendering. |
| Brain.h / Brain.cpp | Neural-network inputs and action scores, fitness, selection, crossover, and mutation. |
| GeneticDodge.vcxproj | Visual Studio project settings and build configuration. |



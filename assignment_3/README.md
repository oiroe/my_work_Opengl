# Assignment 3 - 3D Adventure Game with Skeletal Animation

This project is a 3D game implementation featuring skeletal animation, collision detection, and third-person camera controls.

https://github.com/user-attachments/assets/2138e9c2-45d2-4390-8912-2565be198b04

## Project Structure

The project follows a clean, organized structure:

```
assignment_3/
├── main.cpp           # Main game logic and rendering loop
├── shader.h/.cpp      # Shader compilation and management
├── mesh.h/.cpp        # Mesh data structure with bone support
├── model.h/.cpp       # 3D model loading and animation system
├── glad.c             # OpenGL function loader
├── Makefile           # Build configuration
└── include/           # Required header files
```

## Features

### Skeletal Animation System
The game implements a fully functional skeletal animation system with the following capabilities:
- Support for up to 100 bones per model
- Up to 4 bone influences per vertex for smooth deformations
- Automatic bone weight normalization to prevent distortion
- Keyframe interpolation using quaternion slerp for rotations
- Hierarchical bone transformation computation

### Game Mechanics
- **Player Movement**: WASD controls with camera-relative orientation
- **Collision System**: Sphere-based collision detection for game objects
- **Score Tracking**: Collectible system with real-time score updates
- **Camera System**: Dynamic third-person camera that follows the player

### Graphics Rendering
- **Phong Lighting Model**: Implements ambient, diffuse, and specular components
- **Texture Support**: Multi-path texture loading with automatic fallback
- **Model Loading**: Assimp integration supporting various 3D formats (.dae, .fbx, .obj, etc.)

## Building and Running

The project uses Make for compilation. Ensure you have the required dependencies installed before building.

### Compilation
```bash
make
```

### Execution
```bash
./game
```

### Combined Build and Run
```bash
make run
```

### Clean Build Artifacts
```bash
make clean
```

## Controls

The game uses standard keyboard controls:

| Key | Action |
|-----|--------|
| W | Move forward (camera-relative) |
| S | Move backward |
| A | Move left |
| D | Move right |
| ESC | Exit game |

Movement direction is calculated relative to the camera's orientation, providing intuitive controls regardless of camera position.

## Customization

### Changing the Player Model

To use a different character model, modify the following line in `main.cpp`:

```cpp
Model* playerModel = loadModelFromFile("Swimming.dae");
```

Replace `"Swimming.dae"` with your desired model file. The system supports common formats including .dae, .fbx, and .obj.

### Adjusting Character Scale

Character size can be modified by changing the scale vector:

```cpp
player.scale = glm::vec3(0.01f, 0.01f, 0.01f);
```

Increase values for a larger character, decrease for smaller. All three components can be adjusted independently for non-uniform scaling.

### Adding Obstacles

New obstacles can be added to the scene by inserting into the obstacles vector:

```cpp
obstacles.push_back(GameObject(cubeModel, glm::vec3(x, y, z), radius));
```

Adjust the position coordinates (x, y, z) and collision radius as needed.

## Dependencies

This project requires the following libraries:

- **GLFW 3.x** - Window creation and input handling
- **GLAD** - OpenGL function loader
- **GLM** - Mathematics library for graphics
- **Assimp** - 3D model import library
- **STB_Image** - Image loading library (header-only)

Most Linux distributions provide these through package managers. On Ubuntu/Debian:

```bash
sudo apt-get install libglfw3-dev libassimp-dev libglm-dev
```

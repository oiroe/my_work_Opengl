# Assignment 4: Skeletal Animation with OpenGL

## Overview
This project demonstrates skeletal animation in OpenGL using the Assimp library. It features a 3D character model (Remy) with multiple animations and smooth animation blending between states.

## Features
- **Model Loading**: Load 3D models with skeletal bone structure using Assimp
- **Multiple Animations**: Support for Idle, Walking, Running, and Punch animations
- **Animation Blending**: Smooth transitions between different animation states
- **Keyboard Controls**: Interactive animation switching via keyboard input
- **Camera System**: Free-look camera with mouse and keyboard controls

## Dependencies
- **GLFW3**: Window creation and input handling
- **GLAD**: OpenGL function loader
- **GLM**: Mathematics library for graphics
- **Assimp**: 3D model importing library
- **stb_image**: Texture loading
- **CMake**: Build system (version 3.0+)

## Building the Project

### Linux (WSL)
```bash
cd build
cmake ..
make
```

### Running
```bash
./MyOpenGLProject
# or from the bin directory
cd ../bin
./MyOpenGLProject
```

## Controls

### Animation Controls
- **Key 1**: Switch to Idle animation
- **Key 2**: Switch to Walking animation
- **Key 3**: Switch to Punch animation
- **Key 4**: Switch to Running animation (if enabled)

### Camera Controls
- **W/A/S/D**: Move camera forward/left/backward/right
- **Mouse Movement**: Look around
- **Mouse Scroll**: Zoom in/out
- **ESC**: Close application

## Project Structure
```
assignment_4/
├── skeletal_animation.cpp   # Main application code
├── animator.h                # Animation controller with blending
├── bone.h                    # Bone animation data structures
├── anim_model.vs             # Vertex shader for skeletal animation
├── anim_model.fs             # Fragment shader
├── CMakeLists.txt            # Build configuration
├── include/
│   └── learnopengl/          # Helper classes (Model, Animation, Shader, etc.)
└── resources/
    ├── Remy/                 # Character model
    ├── Idle/                 # Idle animation
    ├── Strut_Walking/        # Walking animation
    ├── Jab_Cross/            # Punch animation
    └── Running/              # Running animation
```

## Technical Details

### Animation System
The project uses a state machine approach for managing animations:
- **IDLE**: Default state
- **IDLE_WALK/WALK_IDLE**: Transition states with blending
- **IDLE_PUNCH/PUNCH_IDLE**: Attack animation transitions
- **WALK**: Continuous walking state

### Bone Transformations
- Supports up to 100 bones per model
- Each vertex can be influenced by up to 4 bones
- Bone transformations are computed on the GPU using vertex shaders

### Animation Blending
- Linear interpolation (lerp) for position and scale
- Spherical linear interpolation (slerp) for rotation quaternions
- Configurable blend rate for smooth transitions

## Shader Details

### Vertex Shader (anim_model.vs)
- Applies bone transformations to vertices
- Handles up to 4 bone influences per vertex
- Computes final vertex position based on weighted bone matrices

### Fragment Shader (anim_model.fs)
- Basic texture sampling
- Outputs textured fragments

## Troubleshooting

### Model/Animation Not Loading
- Ensure all `.dae` files are in the correct `resources/` folders
- Check that `resources/` directory is in the project root
- Verify file paths match exactly (case-sensitive on Linux)

### CMake Configuration Errors
- Install required libraries: `sudo apt install libglfw3-dev libassimp-dev libfreetype6-dev`
- Ensure GLM is installed: `sudo apt install libglm-dev`
- Clear build directory and reconfigure: `rm -rf build/* && cd build && cmake ..`

### Blank Screen or Crash
- Check console for error messages
- Verify shaders are copied to the executable directory
- Ensure model files exist in `resources/` folder

## References
- [LearnOpenGL - Skeletal Animation](https://learnopengl.com/Guest-Articles/2020/Skeletal-Animation)
- [Assimp Documentation](https://assimp.org/index.html)
- [GLFW Documentation](https://www.glfw.org/documentation.html)

## Credits
- Character Model: Mixamo Remy character
- Animations: Mixamo animation library
- Framework: Based on LearnOpenGL tutorials

## License
Educational project for learning purposes.

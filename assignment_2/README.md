# 3D Cube with Dynamic Dual-Light System

An OpenGL demonstration program featuring a rotating 3D cube illuminated by two independently orbiting point lights with distinct colors.

[![Video demo](https://img.youtube.com/vi/MiCzlgjDbCw/0.jpg)](https://youtu.be/MiCzlgjDbCw)

## Overview

This program renders a 3D scene consisting of a rotating cube lit by two colored point lights that orbit around it in opposite directions. When executed, you'll observe:

- A white cube rotating smoothly on both X and Y axes
- A **pink/orange light** orbiting clockwise around the cube's upper hemisphere
- A **blue light** orbiting counter-clockwise around the lower hemisphere
- Dynamic lighting effects that respond realistically to the changing light positions

The result is a visually engaging demonstration of fundamental 3D graphics programming concepts.

## Key Features

### Rendering Capabilities
- **Phong Lighting Model**: Implements diffuse and ambient lighting components for realistic surface illumination
- **Multiple Light Sources**: Supports two independent point lights with configurable colors and positions
- **Depth Testing**: Ensures proper 3D rendering with correct occlusion
- **Smooth Animation**: Time-based transformations create fluid rotation and light movement

### Technical Implementation
- Built with **OpenGL 3.3 Core Profile** for modern graphics pipeline compatibility
- Custom matrix mathematics library (no external dependencies like GLM)
- GLSL vertex and fragment shaders for GPU-accelerated rendering
- Efficient indexed rendering using Element Buffer Objects (EBO)
- Proper vertex attribute configuration with interleaved position and normal data

## Requirements and Setup

### Prerequisites

Before building this program, ensure you have the following:

- **C++ Compiler**: GCC, Clang, or MSVC
- **OpenGL 3.3+**: Most graphics cards from 2010 onwards support this version
- **GLFW3**: Cross-platform library for window management and input handling
- **GLAD**: OpenGL function loader

### Building the Program

**On Linux or WSL (Windows Subsystem for Linux):**
```bash
g++ main.c glad.c -Iinclude -lglfw -ldl -lGL -o main
```

### Running the Program

Execute the compiled binary:
```bash
./main
```

**User Controls:**
- Press `ESC` to exit the application

## Customization Options

This section provides guidance on modifying various aspects of the program's appearance and behavior.

### Modifying Light Colors

Light colors are defined in the main render loop (approximately line 280-281 in `main.c`):

```c
// First light (pink/orange tone)
glUniform3f(glGetUniformLocation(program, "light1Color"), 0.8f, 0.3f, 0.5f);

// Second light (blue tone)
glUniform3f(glGetUniformLocation(program, "light2Color"), 0.2f, 0.5f, 1.0f);
```

RGB values range from 0.0 (off) to 1.0 (full intensity). Here are some common presets:

| Color  | RGB Values |
|--------|------------|
| Red    | `1.0f, 0.0f, 0.0f` |
| Green  | `0.0f, 1.0f, 0.0f` |
| Blue   | `0.0f, 0.0f, 1.0f` |
| White  | `1.0f, 1.0f, 1.0f` |
| Yellow | `1.0f, 1.0f, 0.0f` |
| Purple | `1.0f, 0.0f, 1.0f` |

### Changing the Object Color

The cube's base color is set near line 279:

```c
glUniform3f(glGetUniformLocation(program, "objectColor"), 1.0f, 1.0f, 1.0f);
```

Modify the RGB values to change the cube's material color. This color interacts with the lighting to produce the final appearance.

### Adjusting Rotation Speed

Rotation speeds are controlled by the time multipliers in the transformation calculations (around line 267-269):

```c
rotateX(rotX, time * 0.5f);  // X-axis rotation speed
rotateY(rotY, time * 0.8f);  // Y-axis rotation speed
```

- **Lower values** (e.g., 0.2f): Slower, more contemplative rotation
- **Higher values** (e.g., 2.0f): Faster, more energetic rotation

### Modifying Light Orbit Radius

The distance at which lights orbit is defined around line 276:

```c
float light1X = cos(time) * 2.0f;  // Orbit radius in units
float light1Z = sin(time) * 2.0f;
```

Increase the multiplier (2.0f) for wider orbits, decrease for tighter circles around the cube.

### Adjusting Ambient Lighting

The base illumination level is set in the fragment shader (around line 55):

```c
vec3 ambient = 0.1 * vec3(1.0);  // 10% ambient light
```

- **0.0**: Pure dynamic lighting (dramatic, high-contrast)
- **0.2-0.3**: Balanced visibility with clear lighting effects
- **0.5+**: Brighter scene with reduced lighting contrast

## Technical Architecture

### Rendering Pipeline

The program implements a standard modern OpenGL rendering pipeline:

**1. Vertex Processing**
- The vertex shader transforms each vertex from model space through world space to clip space
- Normal vectors are transformed to world space for lighting calculations
- Position and normal data are passed to the fragment shader via interpolation

**2. Fragment Shading**
- Each pixel calculates illumination from both point lights independently
- Implements Lambertian diffuse reflection: intensity = max(dot(normal, lightDirection), 0.0)
- Combines ambient lighting with both diffuse contributions
- Final color = (ambient + diffuse₁ + diffuse₂) × objectColor

**3. Animation System**
- Time-based transformations using `glfwGetTime()` ensure consistent animation across different hardware
- Light positions calculated via parametric circular motion: position = (cos(θ) × radius, y, sin(θ) × radius)
- Opposite phase offsets (π radians apart) create the counter-rotating effect

### Transformation Hierarchy

The program applies the following transformation sequence:

1. **Model Matrix**: RotationY × RotationX (object-space transformations)
2. **View Matrix**: Translation along negative Z-axis (camera positioning)
3. **Projection Matrix**: Perspective projection (45° field of view, aspect-corrected)

These matrices are computed manually using custom functions, demonstrating the underlying mathematics without library abstractions.

## Code Organization

### Core Components

**Global Data Structures:**
- `vertices[]`: Cube geometry data (24 vertices × 6 floats each: 3 position + 3 normal)
- `indices[]`: Triangle connectivity information (36 indices forming 12 triangles)
- `vertexShaderSource`: GLSL vertex shader program
- `fragmentShaderSource`: GLSL fragment shader program

**Matrix Mathematics Library:**
- `identity()`: Generates 4×4 identity matrix
- `multiply4x4()`: Performs matrix multiplication
- `rotateX()`, `rotateY()`: Create rotation transformation matrices
- `perspective()`: Constructs perspective projection matrix
- `translate()`: Generates translation matrix

**Main Program Flow:**
1. Initialize GLFW context and create application window
2. Load OpenGL function pointers via GLAD
3. Compile, link, and validate shader programs
4. Configure Vertex Array Object (VAO), Vertex Buffer Object (VBO), and Element Buffer Object (EBO)
5. Enter render loop:
   - Process user input (ESC key detection)
   - Clear color and depth buffers
   - Calculate time-based transformations
   - Update shader uniform variables
   - Issue draw call
   - Swap front and back buffers
6. Clean up resources and terminate

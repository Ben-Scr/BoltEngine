# Bolt Engine
Windows Compatible C++ Engine

## Preview
<p align="center">
  <img src="Assets/Documentation/Preview_2.png" width="45%" alt="Preview 1">
  <img src="Assets/Documentation/Preview_3.png" width="45%" alt="Preview 2">
</p>


## External Libraries / API's
- [OpenGL](https://www.opengl.org/) - Rendering API
- [STB](https://github.com/nothings/stb) - Graphics Image Library
- [GLM](https://github.com/g-truc/glm) - Graphics Math Library
- [GLFW](https://github.com/glfw/glfw) - Window Library
- [Box2D](https://github.com/erincatto/box2d) - 2D Physics Library/Engine
- [ENTT](https://github.com/skypjack/entt) - ECS Library
- [Miniaudio](https://github.com/mackron/miniaudio) - Multiplatform Audio Library

## Usage
- Basic 2D Rendering
- Game Development

## How to Build
### CMake (recommended)
1. Install **CMake 3.22+** and a C++23-compatible compiler (Visual Studio 2022 on Windows).
2. Configure the project:
   ```bash
   cmake -S . -B build -G "Visual Studio 17 2022" -A x64
   ```
3. Build all targets (engine + sandbox):
   ```bash
   cmake --build build --config Debug
   ```
4. Run the sandbox executable from:
   `build/Sandbox/Debug/BoltSandbox.exe`

Optional: disable sandbox target when you only want the engine library:
```bash
cmake -S . -B build -DBOLT_BUILD_SANDBOX=OFF
```

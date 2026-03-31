# Bolt
Windows (only) Compatible 2D Game-Engine written in C++

## Preview

### Game

<p align="center">
  <img src="Docs/Preview/Preview_2.png" width="48%" alt="Gameplay – combat system">
  <img src="Docs/Preview/Preview_3.png" width="48%" alt="Gameplay – exploration">
</p>

---

### Editor

<p align="center">
  <img src="Docs/Preview/Preview-Editor.png" width="60%" alt="Level editor interface">
</p>

## External Libraries / API's
- [Bolt-Physics2D](https://github.com/Ben-Scr/Bolt-Physics2D) - Lightweight 2D Physics Library
- [OpenGL](https://www.opengl.org/) - Rendering API
- [STB](https://github.com/nothings/stb) - Graphics Image Library
- [GLM](https://github.com/g-truc/glm) - Graphics Math Library
- [GLFW](https://github.com/glfw/glfw) - Window Library
- [Box2D](https://github.com/erincatto/box2d) - 2D Physics Library
- [ENTT](https://github.com/skypjack/entt) - ECS Library
- [Miniaudio](https://github.com/mackron/miniaudio) - Multiplatform Audio Library

## Usage
- Basic 2D Rendering
- Game Development

## How to Build
### CMake (recommended)
1. Install **CMake 3.22+** and a C++23-compatible compiler.
2. Configure the project:
   ```bash
   cmake -S . -B build -G "Visual Studio 17 2022" -A x64
   ```
3. Build all targets (engine + runtime):
   ```bash
   cmake --build build --config Debug
   ```
4. Run the sandbox executable from:
   `build/Bolt-Runtime/Debug/BoltRuntime.exe`

Optional: disable runtime target when you only want the engine library:
```bash
cmake -S . -B build -DBOLT_BUILD_RUNTIME=OFF
```

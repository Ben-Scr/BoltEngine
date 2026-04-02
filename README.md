# Bolt
Cross-platform 2D game-engine written in C++ (Windows + Linux development support).

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

## External Libraries / APIs
- [Bolt-Physics2D](https://github.com/Ben-Scr/Bolt-Physics2D) - Lightweight 2D Physics Library
- [OpenGL](https://www.opengl.org/) - Rendering API
- [STB](https://github.com/nothings/stb) - Graphics Image Library
- [GLM](https://github.com/g-truc/glm) - Graphics Math Library
- [GLFW](https://github.com/glfw/glfw) - Windowing/input library
- [Box2D](https://github.com/erincatto/box2d) - 2D Physics Library
- [ENTT](https://github.com/skypjack/entt) - ECS Library
- [Miniaudio](https://github.com/mackron/miniaudio) - Multiplatform Audio Library

## Build Requirements
- Git + submodule support
- Python 3
- Premake 5 (`premake5` in `PATH` or under `vendor/bin`)
- C++23-capable compiler

### Linux packages (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install -y build-essential clang python3 premake5 git-lfs \
  libx11-dev libxrandr-dev libxi-dev libxxf86vm-dev libxcursor-dev \
  libxinerama-dev libxext-dev libxrender-dev mesa-common-dev libgl1-mesa-dev
```

## Setup + Generate Build Files
### Windows (Visual Studio)
```bat
scripts\Setup.bat
```
This updates submodules/LFS and generates `vs2022` project files.

### Linux (GNU Make)
```bash
./scripts/Setup.sh
```
This updates submodules/LFS and generates `gmake2` makefiles.

You can also run Premake manually:
```bash
premake5 gmake2   # Linux
premake5 vs2022   # Windows
```

## Build
### Linux
```bash
make config=debug -j$(nproc) Bolt-Engine
make config=debug -j$(nproc) Bolt-Runtime
# Optional if you want the editor
make config=debug -j$(nproc) Bolt-Editor
```

### Windows
Open `Bolt.sln` in Visual Studio 2022 and build `Debug|x64` (or `Release|x64` / `Dist|x64`).

## Notes
- Runtime assets are copied to the runtime output directory after build (`{targetdir}/Assets`).
- Linux builds use GLFW's X11 backend via vendored GLFW sources.

## Known Limitations
- Headless CI or WSL setups without an X11/OpenGL environment can compile but may not run editor/runtime windows.
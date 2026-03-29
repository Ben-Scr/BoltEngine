# 1) Project and Engine Overview

## 1.1 Repository intent

The repository contains a Windows-focused 2D engine in C++ (`Bolt-Engine`) plus two host applications:

- **BoltRuntime**: runtime/sandbox app with a sample scene.  
- **BoltEditor**: editor host with an ImGui-based UI.

The root project builds engine + runtime by default; editor is configured as a separate target.

## 1.2 Repository structure (top level)

- `Bolt-Engine/` – engine DLL (core logic, ECS/scene, rendering, physics, audio, utilities).  
- `Bolt-Runtime/` – sample application consuming the engine.  
- `Bolt-Editor/` – editor application consuming the engine.  
- `External/` – third-party headers/libs (GLFW, Box2D, ENTT, GLM, stb, miniaudio, etc.).  
- `ImGui/` – ImGui sources/includes (integrated through engine build).  
- `Documentation/` – screenshots and technical documentation.

## 1.3 Technology stack / dependencies

Based on build scripts, includes, and implementation:

- **Window/Input/Context**: GLFW  
- **Rendering**: OpenGL 3.3 Core + GLAD  
- **Math**: GLM + custom math/collection types (`Vec2`, `AABB`, `Mat2`, etc.)  
- **ECS**: ENTT (`entt::registry`)  
- **Physics**: Box2D (C API)  
- **Audio**: miniaudio  
- **GUI/Debug UI**: Dear ImGui (GLFW/OpenGL3 backend)

## 1.4 Build organization

- Root `CMakeLists.txt` sets C++23 / C17 and defines option `BOLT_BUILD_RUNTIME`.  
- `Bolt-Engine/CMakeLists.txt` builds shared library **`BoltEngine`**.  
- `Bolt-Runtime/CMakeLists.txt` builds `BoltRuntime`, links `BoltEngine`, and copies assets + engine DLL to output.  
- `Bolt-Editor/CMakeLists.txt` builds `BoltEditor`, links `BoltEngine`, and copies engine DLL to output.

## 1.5 Engine profile (short)

- Main architecture: **Application loop + ECS scene system + subsystem pipeline**.  
- Runtime service references are exposed via `EngineContext` (window, scene manager, renderer, physics, etc.).  
- Scenes are registered/instantiated through `SceneDefinition`; systems implement `ISystem` hooks.  
- Rendering is currently focused on 2D quad/sprite paths (including particle instances).  
- Physics and audio are optional via `ApplicationConfig` flags.


\# 3) Subsystems, Modules, and Core Classes



\## 3.1 Core



\### `Application`



Central orchestration:



\- engine subsystem initialization/shutdown

\- main loop (variable + fixed timestep)

\- static runtime controls (`Quit`, `Pause`, `Reload`, target FPS, run-in-background)

\- host must implement abstract lifecycle hooks



\### `Window`



\- GLFW lifecycle (`Initialize`/`Shutdown`), window creation, fullscreen/decor/resize controls

\- callback bridge from GLFW events to `Input`

\- maintains primary viewport



\### `Input`



\- double-buffered states (`current`/`previous`) for keyboard/mouse

\- API: `GetKey`, `GetKeyDown`, `GetKeyUp` (+ mouse equivalents)

\- additional data: mouse position/delta, scroll, simple movement axis



\### `Time`



\- variable delta time + time scale

\- fixed delta time for physics/fixed updates

\- runtime counters (elapsed real time, simulated elapsed time, frame count)



\### `EngineContext`



\- lightweight runtime service container (raw pointers to active core subsystems)



\## 3.2 Scene \& ECS



\### `SceneManager`



\- registers scenes (`SceneDefinition`)

\- load/unload/reload (additive or exclusive)

\- keeps active scene + loaded scene list

\- initializes built-in component metadata



\### `SceneDefinition`



Declarative scene blueprint containing:



\- system list/factories

\- initialize/load/unload callbacks

\- startup/persistent flags



\### `Scene`



\- owns `entt::registry` and system instances

\- provides entity/component operations and singleton-component helpers

\- registers EnTT construct/destroy handlers for components with engine side effects (physics/camera/particles)



\### `Entity` and `EntityHelper`



\- `Entity`: typed wrapper for component access

\- `EntityHelper`: convenience factories (`CreateCamera2DEntity`, etc.) and helper methods



\## 3.3 Components (selected)



\### General



\- `Transform2DComponent`: position/scale/rotation + transform helpers/matrix generation

\- `RectTransformComponent`, `NameComponent`



\### Graphics



\- `SpriteRendererComponent`: texture handle, color, sorting data

\- `Camera2DComponent`: main camera pointer, orthographic projection, viewport AABB

\- `ParticleSystem2DComponent`: particle state and render data

\- `ImageComponent`



\### Physics



\- `Rigidbody2DComponent`: body type, velocity/mass/damping and transform control

\- `BoxCollider2DComponent` and base `Collider2D`



\### Audio



\- `AudioSourceComponent`: component-level playback control (play/pause/stop/resume)



\### Tags



\- status/semantic markers (`StaticTag`, `DisabledTag`, etc.)



\## 3.4 Rendering subsystem



\### `Renderer2D`



\- initializes mesh + shader program

\- per frame:

&#x20; - clear

&#x20; - gather render instances from scenes

&#x20; - camera viewport culling

&#x20; - sort by layer/order

&#x20; - issue draw calls via quad mesh and sprite shader



\### Supporting classes



\- `Texture2D`, `TextureManager`, `TextureHandle`

\- `Shader`, `SpriteShaderProgram`, `OpenGL`, `QuadMesh`

\- `GizmoRenderer2D` / `Gizmo` for debug overlays



\## 3.5 GUI subsystem



\### `ImGuiRenderer`



\- wraps ImGui context and GLFW/OpenGL3 backend

\- exposes `BeginFrame` / `EndFrame`



\### Systems



\- `EditorUISystem`: dockspace, hierarchy, inspector, stats

\- `ImGuiDebugSystem`: broader debug/runtime controls



\## 3.6 Physics subsystem



\### `PhysicsSystem2D`



\- keeps static main world (`Box2DWorld`)

\- advances physics and processes contact dispatch

\- syncs rigidbody state to transform components



\### `Box2DWorld`



\- wraps world creation, body/shape creation, and stepping

\- associates entity handles through Box2D user data



\### `Physics2D`



\- static query API (overlap circle/box, raycast, all-variants)



\### `CollisionDispatcher`



\- registers callbacks per `b2ShapeId` for begin/end/hit

\- consumes Box2D contact events and dispatches typed `Collision2D`



\## 3.7 Audio subsystem



\### `AudioManager`



\- global miniaudio engine lifecycle

\- handle-based audio load/unload

\- controls playback for `AudioSourceComponent`

\- one-shot queueing/throttling/concurrency limits



\### Data model



\- `AudioHandle`: stable identity for loaded clips

\- `SoundInstance`: active miniaudio sound object



\## 3.8 Utilities / debugging



\- `Logger`: console logging + event hook

\- `Exceptions`: engine error codes, `BoltError`, assert/throw/log macros

\- `File`, `Path`, `Serializer`, `Timer`, `StringHelper`, `Event`

\- `Memory`: optional compile-time allocation tracking



\## 3.9 Observed technical issues (current code state)



These are \*\*observed inconsistencies/rough edges\*\*, not assumed features:



\- `Entity::SetEnabled(bool)` appears logically inverted (`enabled == true` adds `DisabledTag`).

\- Some parameter/variable names look like copy leftovers (`blockTexture` outside texture context).

\- Typo-like API names exist (`GetRRenderLoopDuration`, `overlapBoxAll`).

\- `Box2DWorld` destructor has an early `return;`, so world destruction is skipped there.

\- `Core/Memory.cpp` includes `Memory.h` while repository uses `Memory.hpp`.



These should be considered for future cleanup/refactoring.


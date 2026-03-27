\# 2) Architecture and Runtime Flow



\## 2.1 High-level architecture



```text

Host App (Runtime/Editor)

&#x20; └─ derives from Bolt::Application

&#x20;      ├─ ConfigureScenes()

&#x20;      ├─ Start()/Update()/FixedUpdate()/OnPaused()/OnQuit()

&#x20;      └─ Run()

&#x20;          ├─ Initialize subsystems

&#x20;          ├─ Main loop (variable + fixed step)

&#x20;          └─ Shutdown

```



Main runtime building blocks:



\- `Window` (GLFW window + input callbacks)

\- `Input` (state-based key/mouse tracking)

\- `Time` (delta/fixed delta/time scale/frame count)

\- `SceneManager` (scene and system lifecycle)

\- `Renderer2D`, `ImGuiRenderer`, `GuiRenderer`, `GizmoRenderer2D`

\- `PhysicsSystem2D`

\- `TextureManager`, `AudioManager`



\## 2.2 Application lifecycle (detailed)



`Application::Run()` performs, in order:



1\. Optional single-instance check.

2\. `Initialize()`:

&#x20;  - window + OpenGL

&#x20;  - Renderer2D

&#x20;  - optional gizmo/ImGui/GUI/physics

&#x20;  - TextureManager

&#x20;  - optional AudioManager

&#x20;  - host `ConfigureScenes()` then `SceneManager::Initialize()`

3\. Host `Start()`.

4\. Main loop until window close or quit flag.

5\. `Shutdown()` all subsystems.

6\. Optional self-reload (`Application::Reload()`).



\### Main loop behavior



\- Frame cap uses target FPS and sleep/spin-wait when needed (VSync disabled or paused mode).

\- Per frame:

&#x20; - `Time::Update(deltaTime)`

&#x20; - fixed-step accumulator loop (`BeginFixedFrame()` while enough time is accumulated)

&#x20; - `BeginFrame()` / `EndFrame()`

&#x20; - `glfwPollEvents()`



\### Fixed-step pipeline



\- Host `FixedUpdate()`

\- `SceneManager::FixedUpdateScenes()` (all loaded scenes)

\- `PhysicsSystem2D::FixedUpdate()`



\### Variable frame pipeline



When not paused:



\- AudioManager update (if enabled)

\- Host `Update()`

\- `SceneManager::UpdateScenes()`

\- renderer begin/end, ImGui begin/end, GUI begin/end, gizmo begin/end



When paused:



\- `OnPaused()` is called instead of normal update pipeline.



\## 2.3 Scene/ECS architecture



\### 2.3.1 Scene registration via SceneDefinition



\- Scenes are registered through `SceneManager::RegisterScene("Name")`.

\- A definition stores:

&#x20; - system factories (`AddSystem<T>()`)

&#x20; - callbacks (`OnInitialize`, `OnLoad`, `OnUnload`)

&#x20; - flags (`SetAsStartupScene`, `SetPersistent`)



\### 2.3.2 Instantiation



When loading (`LoadScene`/`LoadSceneAdditive`), a `Scene` instance is created from the definition:



\- dedicated `entt::registry`

\- dedicated system list (`std::vector<std::unique\_ptr<ISystem>>`)

\- callback execution and lifecycle (`Awake`/`Start`)



\### 2.3.3 Multiple loaded scenes



\- `SceneManager` keeps `m\_LoadedScenes` (list of shared\_ptr).

\- Active scene is tracked separately (`m\_ActiveScene`).

\- Update/FixedUpdate/OnGui run across \*\*all loaded scenes\*\*.

\- Non-additive loading unloads previously loaded non-persistent scenes.



\## 2.4 Entity/component model



\- Entity handles are `entt::entity` aliases (`EntityHandle`).

\- `Entity` is a wrapper over handle + registry pointer.

\- Components are attached/accessed through template APIs (`AddComponent<T>`, `GetComponent<T>`, etc.).

\- `Scene::CreateEntity()` adds `Transform2DComponent` by default.



\### Tags



Empty marker components:



\- `IdTag`, `StaticTag`, `DisabledTag`, `DeadlyTag`



\### System base



`ISystem` hooks:



\- `Awake`, `Start`, `Update`, `FixedUpdate`, `OnGui`, `OnDisable`, `OnDestroy`



Systems can be enabled/disabled.



\## 2.5 Data flow across major subsystems



\### 2.5.1 Physics ↔ ECS



\- On add/remove of certain components (`Rigidbody2DComponent`, `BoxCollider2DComponent`), `Scene` automatically creates/destroys Box2D bodies/shapes.

\- Each physics fixed step syncs rigidbody position/rotation back into `Transform2DComponent`.

\- Collision events go through `CollisionDispatcher` (begin/end/hit callback maps by shape).



\### 2.5.2 Rendering ↔ ECS



\- `Renderer2D` iterates all loaded scenes.

\- Reads `Transform2DComponent + SpriteRendererComponent` and `ParticleSystem2DComponent`.

\- Frustum/viewport culling uses `Camera2DComponent::Main()` and `AABB`.

\- Sorting is by `SortingLayer`, then `SortingOrder`.



\### 2.5.3 Input/window ↔ Application



\- GLFW callbacks write into `Application::m\_Input`.

\- At frame end, `Input::Update()` updates previous states and axis values.



\### 2.5.4 Asset managers



\- `TextureManager` and `AudioManager` provide handle-based resource management.

\- Asset paths are relative to asset roots (e.g., `Assets/Textures`, `Assets/Audio`).



\## 2.6 Editor/runtime separation (technical)



\- Both hosts use the same engine library.

\- Separation is mainly through:

&#x20; - different `ApplicationConfig`

&#x20; - different scene configuration (`EditorUISystem` is currently used in both sample hosts)

&#x20; - different window parameters/title/icon behavior

\- There is no separate editor-only engine core target; editor is another engine consumer.


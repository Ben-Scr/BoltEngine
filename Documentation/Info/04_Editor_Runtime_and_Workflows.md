\# 4) Editor/Runtime Separation and Development Workflow



\## 4.1 Host applications



\## BoltRuntime



\- entry: `Bolt-Runtime/Src/Runtime.cpp`

\- defines `Sandbox : Application`

\- registers startup scene `SampleScene`

\- adds `EditorUISystem` to scene systems

\- creates camera entity in `Start()` (`EntityHelper::CreateCamera2DEntity()`)



\## BoltEditor



\- entry: `Bolt-Editor/src/main.cpp`

\- defines `BoltEditorApplication : Application`

\- startup scene: `EditorScene`

\- also uses `EditorUISystem`

\- editor-oriented window setup (1600x900, icon disabled)



\## 4.2 Shared bootstrap mechanism



Both hosts define `Bolt::CreateApplication()`. `EntryPoint.hpp` provides shared Windows `main()`:



1\. call `CreateApplication()`

2\. call `app->Run()`

3\. print exceptions to stdout

4\. delete app instance



\## 4.3 Asset pipeline (current repository)



\- Both editor and runtime include similar `Assets/` trees:

&#x20; - `Audio/`

&#x20; - `Fonts/`

&#x20; - `Shader/`

&#x20; - `Textures/`

\- Runtime CMake copies `Assets` to executable output.

\- Editor CMake only copies engine DLL (no explicit asset-copy command).



\## 4.4 Build workflow (CMake)



\### Recommended (as documented in repository)



```bash

cmake -S . -B build -G "Visual Studio 17 2022" -A x64

cmake --build build --config Debug

```



Optional without runtime:



```bash

cmake -S . -B build -DBOLT\_BUILD\_RUNTIME=OFF

```



\### Running



\- runtime binary is typically at `build/Bolt-Runtime/<Config>/BoltRuntime.exe`

\- editor binary accordingly at `build/Bolt-Editor/<Config>/BoltEditor.exe` (if target is built)



\## 4.5 Typical development loop (inferred from code)



1\. Implement new components/systems in `Bolt-Engine`.

2\. Extend built-in component registration when editor metadata is needed (`RegisterBuiltInComponents`).

3\. Register/configure scenes in host via `SceneManager::RegisterScene`.

4\. Attach systems with `SceneDefinition::AddSystem`.

5\. Place assets in the corresponding `Assets` hierarchy.



\## 4.6 Runtime subsystem interplay



\- \*\*Per frame\*\*: input -> host/scene update -> render/GUI pipeline

\- \*\*Per fixed step\*\*: host fixed update -> scene fixed update -> physics step

\- \*\*Cross-cutting\*\*:

&#x20; - renderer reads ECS component data

&#x20; - physics writes back into transforms

&#x20; - ImGui systems access scene manager/entity state


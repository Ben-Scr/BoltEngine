\# 5) Naming and Coding Conventions (derived from current code)



> Note: the repository currently does not contain `.editorconfig`, `.clang-format`, or a dedicated conventions document. The rules below are inferred from implementation patterns.



\## 5.1 Patterns used consistently



\### File and type naming



\- \*\*File names\*\*: mostly `PascalCase` with suffixes (`Application.hpp`, `PhysicsSystem2D.cpp`).

\- \*\*Classes/structs\*\*: `PascalCase` (`SceneManager`, `Transform2DComponent`).

\- \*\*Namespace\*\*: primarily `namespace Bolt`.



\### Member/static naming



\- instance members mostly use `m\_` prefix (`m\_Window`, `m\_IsInitialized`).

\- static members mostly use `s\_` prefix (`s\_Instance`, `s\_Textures`).

\- constants often use `k\_` prefix (`k\_PausedTargetFrameRate`, `k\_InvalidIndex`).



\### Architecture/code patterns



\- clear header/implementation split (`.hpp`/`.cpp`).

\- lifecycle method pairs are common (`Initialize/Shutdown`, `BeginFrame/EndFrame`).

\- error handling strongly relies on `BOLT\_ASSERT`/`BOLT\_THROW`/`BOLT\_LOG\_ERROR`.



\### API style



\- many subsystems are static managers (`TextureManager`, `AudioManager`, `Physics2D`).

\- ECS interactions use template helpers (`AddComponent<T>`, `GetComponent<T>`, etc.).



\## 5.2 Partially consistent / mixed patterns



\### Comments



\- `Info:` comment prefix appears frequently, but not everywhere.

\- comments are mostly English, with mixed style/detail level.



\### Include ordering



\- many `.cpp` files include `pch` first, but formatting/ordering is not fully uniform.

\- include spacing/style has minor inconsistencies.



\### Boolean API naming



\- many bool getters follow `Is\*`/`Has\*`.

\- some naming remains inconsistent (e.g., `Resizeable` spelling variants).



\## 5.3 Concrete inconsistencies observed



\- \*\*Spelling/style variants\*\*:

&#x20; - `Resizeable` vs common `Resizable`

&#x20; - `overlapBoxAll` (lowercase `o`) next to `OverlapCircleAll`

&#x20; - `GetRRenderLoopDuration` (double `R`)

\- \*\*Behavioral inconsistency\*\*:

&#x20; - `Entity::SetEnabled()` appears inverted

\- \*\*Legacy/copy-paste naming\*\*:

&#x20; - parameter name `blockTexture` used in non-texture contexts

\- \*\*Filename/include mismatch\*\*:

&#x20; - `Memory.cpp` includes `Memory.h`, repository file is `Memory.hpp`

\- \*\*Raw-pointer-heavy service wiring\*\*:

&#x20; - `EngineContext` and some internals rely on raw pointers (likely intentional, but less RAII-oriented)



\## 5.4 Practical convention guidance for new contributions



To stay aligned with existing code while improving consistency:



1\. keep new files/types in \*\*PascalCase\*\*

2\. keep `m\_` for instance members and `s\_` for static members

3\. use explicit bool prefixes for new APIs (`Is/Has/Can/Should`)

4\. normalize new naming toward consistent spelling (e.g., `Resizable`)

5\. fix old inconsistencies in dedicated refactor passes

6\. keep error handling aligned with current Bolt error model/macros



\## 5.5 Transparency



This section documents:



\- \*\*actual patterns\*\* currently present in the codebase

\- \*\*no assumed conventions\*\* that are not supported by repository evidence


#pragma once
#include "Scene/ISystem.hpp"
#include "Core/Export.hpp"

namespace Bolt {

	// ──────────────────────────────────────────────
	//  ScriptSystem — dispatches Start / Update lifecycle calls to all
	//  ScriptComponent instances each frame.
	//
	//  Current implementation:
	//    - Iterates entities with ScriptComponent
	//    - Binds unbound ScriptInstances to their entity
	//    - Calls Start() on the first frame after binding
	//    - Calls Update() every frame thereafter
	//
	//  Future (Mono/CoreCLR):
	//    - On Awake: load the assembly, scan for BoltScript subclasses
	//    - Instantiate managed objects and store GCHandles on ScriptInstance
	//    - Forward Start/Update to managed methods via P/Invoke / internal calls
	//    - Hot-reload: unload AppDomain, reload assembly, rebind handles
	// ──────────────────────────────────────────────

	class BOLT_API ScriptSystem : public ISystem {
	public:
		void Awake(Scene& scene) override;
		void Update(Scene& scene) override;
		void OnDestroy(Scene& scene) override;
	};

} // namespace Bolt

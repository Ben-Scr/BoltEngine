#include "pch.hpp"
#include "Scripting/ScriptSystem.hpp"
#include "Scripting/ScriptComponent.hpp"
#include "Scene/Scene.hpp"
#include "Components/Tags.hpp"

namespace Bolt {

	void ScriptSystem::Awake(Scene& scene) {
		(void)scene;
		// Future: initialize Mono/CoreCLR runtime, load user assembly
		BT_INFO_TAG("ScriptSystem", "Script system initialized (native stub)");
	}

	void ScriptSystem::Update(Scene& scene) {
		auto view = scene.GetRegistry().view<ScriptComponent>(entt::exclude<DisabledTag>);

		for (auto [entity, scriptComp] : view.each()) {
			for (auto& instance : scriptComp.Scripts) {
				// Bind if not yet bound
				if (!instance.IsBound()) {
					instance.Bind(entity);
				}

				// Dispatch Start once
				if (!instance.HasStarted()) {
					instance.MarkStarted();
					// Future: invoke managed Start() via GCHandle
					// For now this is where the Mono interop call would go:
					//   mono_runtime_invoke(startMethod, instance.GetManagedHandle(), nullptr, nullptr);
				}

				// Dispatch Update every frame
				// Future: invoke managed Update() via GCHandle
				//   mono_runtime_invoke(updateMethod, instance.GetManagedHandle(), nullptr, nullptr);
			}
		}
	}

	void ScriptSystem::OnDestroy(Scene& scene) {
		(void)scene;
		// Future: release all managed handles, unload AppDomain
		BT_INFO_TAG("ScriptSystem", "Script system shutdown");
	}

} // namespace Bolt

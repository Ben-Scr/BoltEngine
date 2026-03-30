#pragma once
#include "Core/Export.hpp"

namespace Bolt {
	class Scene;

	class BOLT_API ISystem {
	public:
		friend class Scene;
		friend class SceneDefinition;

		virtual ~ISystem() = default;

		// Info: Gets called every frame
		virtual void Update(Scene& scene) {}

		// Info: Gets called every fixed frame
		virtual void FixedUpdate(Scene& scene) {}

		// Info: Gets called when scene is created
		virtual void Awake(Scene& scene) {}

		// Info: Gets called when scene is created directly after awake
		virtual void Start(Scene& scene) {}

		// Info: Gets called when system is disabled
		virtual void OnDisable(Scene& scene) {}

		// Info: Gets called when system is destroyed
		virtual void OnDestroy(Scene& scene) {}

		// Info: Gets called on imgui frame start (every frame)
		virtual void OnGui(Scene& scene) {}

		bool IsEnabled() const { return m_Enabled; }

	private:
		void SetEnabled(bool enabled, Scene& scene) {
			if (m_Enabled == enabled) {
				return;
			}
			m_Enabled = enabled;
			if (!m_Enabled) {
				OnDisable(scene);
			}
		}

		bool m_Enabled = true;
	};
}
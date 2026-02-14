#pragma once
#include "Core/Core.hpp"

namespace Bolt {
	class Scene;

	class BOLT_API ISystem {
	public:
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

		// Info: Gets called when gui can renders (every frame)
		virtual void OnGui(Scene& scene) {}

		bool IsEnabled() { return m_Enabled; }
	private:
		bool m_Enabled = true;
		friend class Scene;
		friend class SceneDefinition;
	};
}
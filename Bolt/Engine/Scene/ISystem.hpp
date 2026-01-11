#pragma once

namespace Bolt {
	class Scene;

	class ISystem {
	public:
		// Info: Gets called every frame
		virtual void Update() {}

		// Info: Gets called every fixed frame
		virtual void FixedUpdate() {}

		// Info: Gets called when scene is created
		virtual void Awake() {}

		// Info: Gets called when scene is created
		virtual void Start() {}
		virtual void OnDisable() {}
		virtual void OnDestroy() {}
		virtual void OnPaused() {}

		virtual void OnApplicationInitialize() {}
		virtual void OnApplicationExit() {}

		bool IsEnabled() { return m_Enabled; }
		virtual ~ISystem() = default;

		void SetScene(std::weak_ptr<Scene> scene) { m_Scene = scene; }
		Scene& GetScene() { return *m_Scene.lock(); }

	private:
		std::weak_ptr<Scene> m_Scene;
		bool m_Enabled = true;
		friend class Scene;
		friend class SceneDefinition;
	};
}
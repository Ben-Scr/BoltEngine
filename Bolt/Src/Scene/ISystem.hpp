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

		// Info: Gets called when scene is created directly after awake
		virtual void Start() {}

		// Info: Gets called when system is disabled
		virtual void OnDisable() {}

		// Info: Gets called when system is destroyed
		virtual void OnDestroy() {}

		// Info: Gets called when gui can renders (every frame)
		virtual void OnGui() {}

		// Info: Gets called when Application is paused (default: 10fps)
		virtual void OnApplicationPaused() {}

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
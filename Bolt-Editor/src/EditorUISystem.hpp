#pragma once
#include "Components/Components.hpp"
#include "Core/Application.hpp"
#include "Core/Time.hpp"
#include "Graphics/Renderer2D.hpp"
#include "Scene/EntityHelper.hpp"
#include "Scene/ISystem.hpp"
#include "Scene/Scene.hpp"
#include "Scene/SceneDefinition.hpp"
#include "Scene/SceneManager.hpp"


namespace Bolt {
	class EditorUISystem : public ISystem {
	public:
		void OnGui(Scene& scene) override;

	private:
		EntityHandle m_SelectedEntity = entt::null;
		int m_EntityCounter = 0;

		void DrawDockspace();
		void DrawMenuBar(Scene& scene);
		void DrawHierarchy(Scene& scene);
		void DrawInspector(Scene& scene);
		void DrawStats();
		void CreateEntity(Scene& scene);
	};
}
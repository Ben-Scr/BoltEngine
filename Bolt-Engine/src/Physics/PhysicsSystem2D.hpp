#pragma once
#include <optional>
#include "Physics/Box2DWorld.hpp"

namespace Bolt {

	class PhysicsSystem2D {
	public:
		void FixedUpdate(float dt);

		void Initialize();
		void Shutdown();

		static Box2DWorld& GetMainPhysicsWorld() { return s_MainWorld.value(); }
		static bool IsEnabled() { return s_IsEnabled; };
		static void SetEnabled(bool enabled) { s_IsEnabled = enabled; }
	private:
		static std::optional<Box2DWorld> s_MainWorld;
		static bool s_IsEnabled;
	};
}
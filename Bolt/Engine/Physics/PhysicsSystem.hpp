#pragma once

namespace Bolt {
	class Box2DWorld;

	class PhysicsSystem {
	public:
		void FixedUpdate(float dt);
		static Box2DWorld& GetMainPhysicsWorld() { return s_MainWorld; }
		static bool IsEnabled() { return s_IsEnabled; };
		static void SetEnabled(bool enabled) { s_IsEnabled = enabled; }
	private:
		static Box2DWorld s_MainWorld;
		static bool s_IsEnabled;
	};
}
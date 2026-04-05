#pragma once
#include <optional>

namespace Bolt {
	class Box2DWorld;

	class PhysicsSystem2D {
	public:
		void FixedUpdate(float dt);

		void Initialize();
		void Shutdown();

		// F-14: optional avoids constructing Box2DWorld (and its b2WorldId) at static-init time,
		// before the application has started and before any OpenGL / Box2D context exists.
		static Box2DWorld& GetMainPhysicsWorld() { return s_MainWorld.value(); }
		static bool IsEnabled() { return s_IsEnabled; };
		static void SetEnabled(bool enabled) { s_IsEnabled = enabled; }
	private:
		static std::optional<Box2DWorld> s_MainWorld;
		static bool s_IsEnabled;
	};
}
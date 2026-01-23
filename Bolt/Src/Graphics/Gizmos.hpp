#pragma once
#include "../Collections/AABB.hpp"
#include "../Collections/Color.hpp"
#include <vector>

namespace Bolt {
	struct Square { Vec2 Center, HalfExtents; float Radiant; Color Color; };
	struct Circle { Vec2 Center; float Radius; int Segments; Color Color; };
	struct Line { Vec2 Start; Vec2 End; Color Color; };

	struct Box { Vec3 Center, HalfExtents; float Radiant; Color Color; };
	struct Sphere { Vec3 Center; float Radius; int Segments; Color Color; };
	struct Line3D { Vec3 Start; Vec3 End; Color Color; };

	class Gizmo {
	public:
		// 2D
		static void DrawSquare(const Vec2& center, const Vec2& scale, float degrees);
		static void DrawLine(const Vec2& start, const Vec2& end);
		static void DrawCircle(const Vec2& center, float radius, int segments = 32);

		// 3D
		static void DrawBox(const Vec3& center, const Vec3& scale, const Vec3& rotation);
		static void DrawSphere(const Vec3& center, const float radius);
		static void DrawLine3D(const Vec3& start, const Vec3& end);

		static void SetEnabled(bool enabled) { s_IsEnabled = enabled; }
		static bool IsEnabled() { return s_IsEnabled; }

		static void SetColor(const Color& color) { s_Color = color; }
		static Color GetColor() { return s_Color; }

		static void SetMaxVertices(size_t maxVertices) { s_MaxVertices = maxVertices; };
		static size_t GetMaxVertices() { return s_MaxVertices; }

		static size_t GetRegisteredVertices() { return s_RegisteredVertices; }

		static void Clear();
		static AABB s_CamViewportAABB;
	private:
		static bool s_IsEnabled;

		static  Color s_Color;;

		static const size_t k_BoxVertices = 4;
		static const size_t k_LineVertices = 1;

		static std::vector<Square> s_Squares;
		static std::vector<Circle> s_Circles;
		static std::vector<Line> s_Lines;

		static size_t s_MaxVertices;
		static size_t s_RegisteredVertices;

		friend class GizmoRenderer2D;
		friend class Renderer2D;
	};
}
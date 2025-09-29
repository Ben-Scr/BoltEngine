#pragma once
#include "../Collections/AABB.hpp"
#include "../Collections/Color.hpp"
#include <vector>

namespace Bolt {
	struct Box { Vec2 Center, HalfExtents; float Radiant; Color Color; };
	struct Circle { Vec2 Center; float Radius; int Segments; Color Color; };
	struct Line { Vec2 Start; Vec2 End; Color Color; };

	class Gizmos {
	public:
		static void DrawBox(const Vec2& center, const Vec2& scale, float degrees);
		static void DrawLine(const Vec2& start, const Vec2& end);
		static void DrawCircle(const Vec2& center, float radius, int segments = 32);

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

		static std::vector<Box> s_Boxes;
		static std::vector<Circle> s_Circles;
		static std::vector<Line> s_Lines;

		static size_t s_MaxVertices;
		static size_t s_RegisteredVertices;

		friend class GizmoRenderer;
		friend class Renderer2D;
	};
}
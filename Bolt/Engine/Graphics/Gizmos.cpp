#include "../pch.hpp"
#include "Gizmos.hpp"
#include "Camera2D.hpp"

namespace Bolt {
	std::vector<Square> Gizmos::s_Squares;
	std::vector<Circle> Gizmos::s_Circles;
	std::vector<Line> Gizmos::s_Lines;
	size_t Gizmos::s_MaxVertices = 100000;
	size_t Gizmos::s_RegisteredVertices = 0;

	bool Gizmos::s_IsEnabled = true;
	Color Gizmos::s_Color = { 0.f, 1.f, 0.f, 1.f };

	void Gizmos::DrawCircle(const Vec2& center, float radius, int segments) {
		if (!Camera2D::Main() || !s_IsEnabled || s_RegisteredVertices + segments >= s_MaxVertices)
			return;

		AABB circleAABB = AABB::Create(center, Vec2(radius));
		if (!AABB::Intersects(circleAABB, Camera2D::Main()->GetViewportAABB()))
			return;

		s_RegisteredVertices += segments;
		s_Circles.emplace_back(Circle{ center,radius,segments, s_Color });
	}

	void Gizmos::DrawSquare(const Vec2& center, const Vec2& scale, float degrees) {
		if (!Camera2D::Main() || !s_IsEnabled || s_RegisteredVertices + k_BoxVertices >= s_MaxVertices)
			return;

		float radiant = Radians<float>(degrees);
		AABB boxAABB = AABB::IsAxisAligned(radiant) ? AABB::Create(center, scale / 2.f) : AABB::Create(center, scale / 2.f, degrees);
		if (!AABB::Intersects(boxAABB, Camera2D::Main()->GetViewportAABB()))
			return;

		s_RegisteredVertices += k_BoxVertices;
		s_Squares.emplace_back(Square{ center, scale / 2.f, Radians<float>(degrees), s_Color });
	}

	void Gizmos::DrawLine3D(const Vec2& start, const Vec2& end) {
		if (!Camera2D::Main() || !s_IsEnabled || s_RegisteredVertices + k_LineVertices >= s_MaxVertices)
			return;

		const auto camAABB = Camera2D::Main()->GetViewportAABB();
		if (!AABB::Contains(camAABB, start) && !AABB::Contains(camAABB, end))
			return;

		s_RegisteredVertices += k_LineVertices;
		s_Lines.emplace_back(Line{ start, end, s_Color });
	}

	void Gizmos::Clear() {
		s_Squares.clear();
		s_Lines.clear(); 
		s_Circles.clear();
		s_RegisteredVertices = 0; 
	}
}

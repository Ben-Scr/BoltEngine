#include "../pch.hpp"
#include "../Components/BoxCollider2D.hpp"
#include "../Scene/Scene.hpp"

namespace Bolt {
	void BoxCollider2D::SetScale(const Vec2& scale, const Scene& scene) {
		Vec2 center = this->GetCenter();
		Transform2D tr = scene.GetComponent<Transform2D>(m_EntityHandle);
		b2Polygon polygon = b2MakeOffsetBox(tr.Scale.x * scale.x * 0.5f, tr.Scale.y * scale.y * 0.5f, b2Vec2(center.x, center.y), tr.GetB2Rotation());
		b2Shape_SetPolygon(m_ShapeId, &polygon);
	}

	Vec2 BoxCollider2D::GetScale() {
		b2ShapeType shapeType = b2Shape_GetType(m_ShapeId);

		if (shapeType == b2_polygonShape) {
			b2Polygon polygon = b2Shape_GetPolygon(m_ShapeId);

			if (polygon.count == 4) {
				Vec2 size = Vec2(
					polygon.vertices[2].x - polygon.vertices[0].x,
					polygon.vertices[2].y - polygon.vertices[0].y
				);
				return size;
			}
			else {
				throw std::runtime_error("This boxshape polygon count equals " + std::to_string(polygon.count) + " instead of 4");
			}
		}
		else {
			throw std::runtime_error("This box shapetype isn't type of b2_polygonShape");
		}

		return Vec2(1.0f);
	}

	Vec2 BoxCollider2D::GetLocalScale(const Scene& scene) {
		Transform2D tr = scene.GetComponent<Transform2D>(m_EntityHandle);

		b2ShapeType shapeType = b2Shape_GetType(m_ShapeId);
		if (shapeType == b2_polygonShape) {
			b2Polygon polygon = b2Shape_GetPolygon(m_ShapeId);

			if (polygon.count == 4) {
				Vec2 size = Vec2(
					polygon.vertices[2].x - polygon.vertices[0].x,
					polygon.vertices[2].y - polygon.vertices[0].y
				);
				size += Vec2{ 1.f };
				return size - tr.Scale;
			}
			else {
				throw std::runtime_error("This boxshape polygon count equals " + std::to_string(polygon.count) + " instead of 4");
			}
		}
		else {
			throw std::runtime_error("This box shapetype isn't type of b2_polygonShape");
		}

		return Vec2(1.0f);
	}

	void BoxCollider2D::UpdateScale(const Scene& scene) {
		Vec2 center = this->GetCenter();
		Transform2D tr = scene.GetComponent<Transform2D>(m_EntityHandle);

		b2Polygon polygon = b2MakeOffsetBox(tr.Scale.x * 0.5f, tr.Scale.y * 0.5f, b2Vec2(center.x, center.y), tr.GetB2Rotation());
		b2Shape_SetPolygon(m_ShapeId, &polygon);
	}

	void BoxCollider2D::SetCenter(const Vec2& center, const Scene& scene) {
		Transform2D tr = scene.GetComponent<Transform2D>(m_EntityHandle);
		b2Polygon polygon = b2MakeOffsetBox(tr.Scale.x * 0.5f, tr.Scale.y * 0.5f, b2Vec2(center.x, center.y), b2Rot_identity);
		b2Shape_SetPolygon(m_ShapeId, &polygon);
	}

	Vec2 BoxCollider2D::GetCenter() {
		b2Polygon polygon = b2Shape_GetPolygon(m_ShapeId);
		return Vec2(polygon.centroid.x, polygon.centroid.y);
	}
}
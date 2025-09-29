#include "../pch.hpp"
#include "../Components/Transform2D.hpp"

namespace Bolt {

	Transform2D Transform2D::FromPosition(const Vec2& pos) {
		Transform2D tr;
		tr.Position = pos;
		return tr;
	}
	Transform2D Transform2D::FromScale(const Vec2& scale) {
		Transform2D tr;
		tr.Scale = scale;
		return tr;
	}


	float Transform2D::GetRotationDegrees() const { return Degrees(Rotation); }
	glm::mat3 Transform2D::GetModelMatrix() const {
		const float s = glm::sin(Rotation);
		const float c = glm::cos(Rotation);

		// Rotation matrix in 3x3 form:
		glm::mat3 rotMatrix{
			{ c,  s, 0.0f },
			{ -s, c, 0.0f },
			{ 0.0f, 0.0f, 1.0f }
		};

		// Scaling matrix in 3x3 form:
		glm::mat3 scaleMat{
			{ Scale.x, 0.0f,   0.0f },
			{ 0.0f,   Scale.y, 0.0f },
			{ 0.0f,   0.0f,    1.0f }
		};

		// Corrected Translation matrix in 3x3 form:
		glm::mat3 transMat{
			{ 1.0f, 0.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },
			{ Position.x, Position.y, 1.0f }
		};

		// Combine them: translation * rotation * scale
		return transMat * rotMatrix * scaleMat;
	}

	b2Rot Transform2D::GetB2Rotation() const {
		return  b2Rot(Cos(Rotation), Sin(Rotation));
	}
}
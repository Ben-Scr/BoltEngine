#include "../pch.hpp"
#include "../Components/Transform.hpp"

namespace Bolt {

	Transform Transform::FromPosition(const Vec3& pos) {
		Transform tr;
		tr.Position = pos;
		return tr;
	}
	Transform Transform::FromScale(const Vec3& scale) {
		Transform tr;
		tr.Scale = scale;
		return tr;
	}


	glm::mat3 Transform2D::GetModelMatrix() const {
		const float s = glm::sin(Rotation);
		const float c = glm::cos(Rotation);

		glm::mat3 rotMatrix{
			{ c,  s, 0.0f },
			{ -s, c, 0.0f },
			{ 0.0f, 0.0f, 1.0f }
		};


		glm::mat3 scaleMat{
			{ Scale.x, 0.0f,   0.0f },
			{ 0.0f,   Scale.y, 0.0f },
			{ 0.0f,   0.0f,    1.0f }
		};

		glm::mat3 transMat{
			{ 1.0f, 0.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },
			{ Position.x, Position.y, 1.0f }
		};


		return transMat * rotMatrix * scaleMat;
	}
}
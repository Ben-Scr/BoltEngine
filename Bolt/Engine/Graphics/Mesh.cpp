#include "../pch.hpp"
#include "Mesh.hpp"

#include <glm/gtc/constants.hpp>

#include <cmath>

namespace Bolt {
	namespace {
		constexpr GLuint POSITION_LOCATION = 0;
		constexpr GLuint NORMAL_LOCATION = 1;
		constexpr GLuint TEXCOORD_LOCATION = 2;
	}

	Mesh::Mesh(Mesh&& other) noexcept {
		*this = std::move(other);
	}

	Mesh& Mesh::operator=(Mesh&& other) noexcept {
		if (this != &other) {
			Release();
			m_VAO = other.m_VAO;
			m_VBO = other.m_VBO;
			m_EBO = other.m_EBO;
			m_VertexCount = other.m_VertexCount;
			m_IndexCount = other.m_IndexCount;

			other.m_VAO = 0;
			other.m_VBO = 0;
			other.m_EBO = 0;
			other.m_VertexCount = 0;
			other.m_IndexCount = 0;
		}
		else {
			glDrawArrays(GL_TRIANGLES, 0, m_VertexCount);
		}
	}

	Mesh::~Mesh() {
		Release();
	}

	void Mesh::Shutdown() {
		Release();
	}

	void Mesh::Release() {
		if (m_EBO) {
			glDeleteBuffers(1, &m_EBO);
			m_EBO = 0;
		}
		if (m_VBO) {
			glDeleteBuffers(1, &m_VBO);
			m_VBO = 0;
		}
		if (m_VAO) {
			glDeleteVertexArrays(1, &m_VAO);
			m_VAO = 0;
		}
		m_VertexCount = 0;
		m_IndexCount = 0;
	}

	void Mesh::SetData(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices) {
		if (vertices.empty()) {
			Logger::Warning("Mesh", "Attempted to upload mesh with no vertices");
			return;
		}

		if (m_VAO == 0) {
			glGenVertexArrays(1, &m_VAO);
			glGenBuffers(1, &m_VBO);
			glGenBuffers(1, &m_EBO);
		}

		glBindVertexArray(m_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(MeshVertex)), vertices.data(), GL_STATIC_DRAW);

		if (!indices.empty()) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(uint32_t)), indices.data(), GL_STATIC_DRAW);
			m_IndexCount = static_cast<GLsizei>(indices.size());
		}
		else {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			m_IndexCount = 0;
		}

		m_VertexCount = static_cast<GLsizei>(vertices.size());

		glEnableVertexAttribArray(POSITION_LOCATION);
		glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), reinterpret_cast<void*>(offsetof(MeshVertex, Position)));

		glEnableVertexAttribArray(NORMAL_LOCATION);
		glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), reinterpret_cast<void*>(offsetof(MeshVertex, Normal)));

		glEnableVertexAttribArray(TEXCOORD_LOCATION);
		glVertexAttribPointer(TEXCOORD_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), reinterpret_cast<void*>(offsetof(MeshVertex, TexCoord)));

		glBindVertexArray(0);
	}

	void Mesh::Bind() const {
		glBindVertexArray(m_VAO);
	}

	void Mesh::Unbind() const {
		glBindVertexArray(0);
	}

	void Mesh::Draw() const {
		if (!IsValid()) {
			return;
		}

		if (m_IndexCount > 0) {
			glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
		}
		else {
			glDrawArrays(GL_TRIANGLES, 0, m_VertexCount);
		}
	}

	std::shared_ptr<Mesh> Mesh::Cube() {
		static std::weak_ptr<Mesh> cubeMeshWeak;
		std::shared_ptr<Mesh> cube = cubeMeshWeak.lock();
		if (cube) {
			return cube;
		}

		cube = std::make_shared<Mesh>();

		std::vector<MeshVertex> vertices;
		vertices.reserve(24);
		std::vector<uint32_t> indices;
		indices.reserve(36);

		const glm::vec2 baseUVs[4] = {
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},
			{0.0f, 1.0f}
		};

		auto addFace = [&](const glm::vec3(&positions)[4], const glm::vec3& normal) {
			const uint32_t startIndex = static_cast<uint32_t>(vertices.size());
			for (int i = 0; i < 4; ++i) {
				MeshVertex vertex;
				vertex.Position = positions[i];
				vertex.Normal = normal;
				vertex.TexCoord = baseUVs[i];
				vertices.push_back(vertex);
			}

			indices.push_back(startIndex + 0);
			indices.push_back(startIndex + 1);
			indices.push_back(startIndex + 2);
			indices.push_back(startIndex + 0);
			indices.push_back(startIndex + 2);
			indices.push_back(startIndex + 3);
			};

		const float half = 0.5f;

		const glm::vec3 front[4] = {
			{-half, -half,  half},
			{ half, -half,  half},
			{ half,  half,  half},
			{-half,  half,  half},
		};
		const glm::vec3 back[4] = {
			{ half, -half, -half},
			{-half, -half, -half},
			{-half,  half, -half},
			{ half,  half, -half},
		};
		const glm::vec3 left[4] = {
			{-half, -half, -half},
			{-half, -half,  half},
			{-half,  half,  half},
			{-half,  half, -half},
		};
		const glm::vec3 right[4] = {
			{ half, -half,  half},
			{ half, -half, -half},
			{ half,  half, -half},
			{ half,  half,  half},
		};
		const glm::vec3 top[4] = {
			{-half,  half,  half},
			{ half,  half,  half},
			{ half,  half, -half},
			{-half,  half, -half},
		};
		const glm::vec3 bottom[4] = {
			{-half, -half, -half},
			{ half, -half, -half},
			{ half, -half,  half},
			{-half, -half,  half},
		};

		addFace(front, { 0.0f,  0.0f,  1.0f });
		addFace(back, { 0.0f,  0.0f, -1.0f });
		addFace(left, { -1.0f,  0.0f,  0.0f });
		addFace(right, { 1.0f,  0.0f,  0.0f });
		addFace(top, { 0.0f,  1.0f,  0.0f });
		addFace(bottom, { 0.0f, -1.0f,  0.0f });

		cube->SetData(vertices, indices);
		cubeMeshWeak = cube;
		return cube;
	}

	std::shared_ptr<Mesh> Mesh::Sphere() {
		static std::weak_ptr<Mesh> sphereMeshWeak;
		std::shared_ptr<Mesh> sphere = sphereMeshWeak.lock();
		if (sphere) {
			return sphere;
		}

		sphere = std::make_shared<Mesh>();

		constexpr uint32_t latitudeSegments = 16;
		constexpr uint32_t longitudeSegments = 32;
		constexpr float radius = 0.5f;

		std::vector<MeshVertex> vertices;
		vertices.reserve((latitudeSegments + 1) * (longitudeSegments + 1));
		std::vector<uint32_t> indices;
		indices.reserve(latitudeSegments * longitudeSegments * 6);

		for (uint32_t lat = 0; lat <= latitudeSegments; ++lat) {
			const float v = static_cast<float>(lat) / static_cast<float>(latitudeSegments);
			const float theta = v * glm::pi<float>();
			const float sinTheta = std::sin(theta);
			const float cosTheta = std::cos(theta);

			for (uint32_t lon = 0; lon <= longitudeSegments; ++lon) {
				const float u = static_cast<float>(lon) / static_cast<float>(longitudeSegments);
				const float phi = u * glm::two_pi<float>();
				const float sinPhi = std::sin(phi);
				const float cosPhi = std::cos(phi);

				MeshVertex vertex;
				glm::vec3 normal{ sinTheta * cosPhi, cosTheta, sinTheta * sinPhi };
				vertex.Position = normal * radius;
				vertex.Normal = glm::normalize(normal);
				vertex.TexCoord = { u, 1.0f - v };
				vertices.push_back(vertex);
			}
		}

		const uint32_t ringStride = longitudeSegments + 1;
		for (uint32_t lat = 0; lat < latitudeSegments; ++lat) {
			for (uint32_t lon = 0; lon < longitudeSegments; ++lon) {
				const uint32_t current = lat * ringStride + lon;
				const uint32_t next = current + ringStride;

				indices.push_back(current);
				indices.push_back(next);
				indices.push_back(current + 1);

				indices.push_back(next);
				indices.push_back(next + 1);
				indices.push_back(current + 1);
			}
		}

		sphere->SetData(vertices, indices);
		sphereMeshWeak = sphere;
		return sphere;
	}

	std::shared_ptr<Mesh> Mesh::Quad() {
		static std::weak_ptr<Mesh> quadMeshWeak;
		std::shared_ptr<Mesh> quad = quadMeshWeak.lock();
		if (quad) {
			return quad;
		}

		quad = std::make_shared<Mesh>();

		const float half = 0.5f;
		std::vector<MeshVertex> vertices = {
			{ {-half, -half, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f} },
			{ { half, -half, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} },
			{ { half,  half, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} },
			{ {-half,  half, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} },
		};

		std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 };

		quad->SetData(vertices, indices);
		quadMeshWeak = quad;
		return quad;
	}
}
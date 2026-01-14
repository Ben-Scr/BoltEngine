#pragma once
#include "../Components/Transform2D.hpp"
#include "../Graphics/TextureHandle.hpp"
#include "../Collections/Color.hpp"
#include <variant>

namespace Bolt {
	struct Particle {
		Transform2D Transform;
		Color Color;
		float LifeTime;
		Vec2 Velocity;
	};

	enum class Space {
		Local,
		World
	};


	struct Burst {
		uint32_t Count{ 10 };
		float Interval{ 1.f };
		float TimeUntilNext{ 0.f };
	};

	struct RenderingSettings {
		uint32_t MaxParticles{ 1000 };
		Color Color{ Color::White() };
		short SortingOrder{ 0 };
		uint8_t SortingLayer{ 0 };
	};

	struct ParticleSettings {
		float LifeTime{ 1.f };
		float Speed{ 5.f };
		Vec2 Gravity{ 0.f };
		bool UseGravity{ false };
		bool UseRandomColors{ false };
		float Scale{ 1.f };
	};

	struct EmissionSettings {
		uint16_t EmitOverTime{ 10 };
		uint16_t RateOverDistance{ 0 };
		Space EmissionSpace{ Space::World };
	};



	struct CircleParams { float Radius = 1.f; bool IsOnCircle = false; };
	struct SquareParams { Vec2 HalfExtends{ 1.f,1.f }; };

	struct RandomColorParams { Color From{ 1.f,1.f,1.f,1.f }; Color To{ 1.f,1.f,1.f,0.f }; };
	struct RandomScaleParams { Vec2 From{ 1.f,1.f }; Vec2 To{ 1.f,1.f }; };
	struct RandomRotationParams { float From{ 0.f }; float To{ 0.f }; };

	using ShapeParams = std::variant<CircleParams, SquareParams>;


	class ParticleSystem2D {
		friend class Scene;
		friend class Renderer2D;
		friend class ParticleUpdateSystem;

	public:
		ParticleSystem2D() = default;
		void SetTexture(const TextureHandle& texture) { m_TextureHandle = texture; }
		const TextureHandle& GetTextureHandle() const { return m_TextureHandle; }
		void Emit(size_t count);
		void AddBurst(const Burst& burst) { m_Bursts.push_back(burst); }
		std::span<const Particle> GetParticles() const noexcept { return m_Particles; }
		bool IsPlaying() const { return m_IsEmitting; }

		// Info: Enables both emitting and simulating
		void Play() { m_IsEmitting = true; m_IsSimulating = true; }

		// Info: Disables/Pauses emitting and simulating
		void Pause() { m_IsEmitting = false; m_IsSimulating = false; }

		// Info: Disables emitting but keeps simulating
		void Stop() { m_IsEmitting = false; }

		void SetIsSimulating(bool enabled) { m_IsSimulating = enabled; }
		void SetIsEmitting(bool enabled) { m_IsEmitting = enabled; }

		bool IsEmitting() const { return m_IsEmitting; }
		bool IsSimulating() const { return m_IsSimulating; }

		void Clear() { m_Particles.clear(); m_Bursts.clear();  m_EmitAccumulator = 0.f; }

		ParticleSettings ParticleSettings;
		EmissionSettings EmissionSettings;
		ShapeParams Shape = CircleParams{ 1.f };
		RenderingSettings RenderingSettings;

	private:
		void Update();
		std::vector<Particle> m_Particles;
		std::vector<Burst> m_Bursts;

		Transform2D* m_EmitterTransform{ nullptr };
		float m_EmitAccumulator{ 0.0f };
		bool m_IsEmitting{ false };
		bool m_IsSimulating{ false };
		TextureHandle m_TextureHandle;
	};
}
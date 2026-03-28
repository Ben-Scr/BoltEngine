#include <pch.hpp>

#include "EditorProjectSerializer.hpp"

#include <optional>

#include "Components/Components.hpp"
#include "Utils/File.hpp"
#include "Utils/Cereal.hpp"

#include <cereal/types/optional.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

namespace Bolt {
	namespace {
		struct SerializableVec2 {
			float x = 0.0f;
			float y = 0.0f;

			template <typename Archive>
			void serialize(Archive& archive) {
				archive(CEREAL_NVP(x), CEREAL_NVP(y));
			}
		};

		struct SerializableColor {
			float r = 1.0f;
			float g = 1.0f;
			float b = 1.0f;
			float a = 1.0f;

			template <typename Archive>
			void serialize(Archive& archive) {
				archive(CEREAL_NVP(r), CEREAL_NVP(g), CEREAL_NVP(b), CEREAL_NVP(a));
			}
		};

		struct SerializableTextureHandle {
			uint16_t index = 0;
			uint16_t generation = 0;

			template <typename Archive>
			void serialize(Archive& archive) {
				archive(CEREAL_NVP(index), CEREAL_NVP(generation));
			}
		};

		struct SerializableTransform2D {
			SerializableVec2 position{};
			SerializableVec2 scale{ 1.0f, 1.0f };
			float rotation = 0.0f;

			template <typename Archive>
			void serialize(Archive& archive) {
				archive(CEREAL_NVP(position), CEREAL_NVP(scale), CEREAL_NVP(rotation));
			}
		};

		struct SerializableCamera2D {
			float orthographicSize = 5.0f;
			float zoom = 1.0f;

			template <typename Archive>
			void serialize(Archive& archive) {
				archive(CEREAL_NVP(orthographicSize), CEREAL_NVP(zoom));
			}
		};

		struct SerializableSpriteRenderer {
			short sortingOrder = 0;
			uint32_t sortingLayer = 0;
			SerializableTextureHandle textureHandle{};
			SerializableColor color{};

			template <typename Archive>
			void serialize(Archive& archive) {
				archive(CEREAL_NVP(sortingOrder), CEREAL_NVP(sortingLayer), CEREAL_NVP(textureHandle), CEREAL_NVP(color));
			}
		};

		struct SerializableImage {
			SerializableTextureHandle textureHandle{};
			SerializableColor color{};

			template <typename Archive>
			void serialize(Archive& archive) {
				archive(CEREAL_NVP(textureHandle), CEREAL_NVP(color));
			}
		};

		struct SerializableParticleSystem2D {
			bool isEmitting = false;
			bool isSimulating = false;
			SerializableTextureHandle textureHandle{};

			template <typename Archive>
			void serialize(Archive& archive) {
				archive(CEREAL_NVP(isEmitting), CEREAL_NVP(isSimulating), CEREAL_NVP(textureHandle));
			}
		};

		struct SerializableAudioSource {
			float volume = 1.0f;
			float pitch = 1.0f;
			bool loop = false;
			uint32_t audioHandle = 0;

			template <typename Archive>
			void serialize(Archive& archive) {
				archive(CEREAL_NVP(volume), CEREAL_NVP(pitch), CEREAL_NVP(loop), CEREAL_NVP(audioHandle));
			}
		};

		struct SerializableComponents {
			std::optional<SerializableTransform2D> Transform2D;
			std::optional<SerializableCamera2D> Camera2D;
			std::optional<SerializableSpriteRenderer> SpriteRenderer;
			std::optional<SerializableImage> Image;
			std::optional<SerializableParticleSystem2D> ParticleSystem2D;
			std::optional<SerializableAudioSource> AudioSource;
			bool StaticTag = false;
			bool DisabledTag = false;
			bool DeadlyTag = false;
			bool IdTag = false;

			template <typename Archive>
			void serialize(Archive& archive) {
				archive(CEREAL_NVP(Transform2D), CEREAL_NVP(Camera2D), CEREAL_NVP(SpriteRenderer), CEREAL_NVP(Image),
					CEREAL_NVP(ParticleSystem2D), CEREAL_NVP(AudioSource), CEREAL_NVP(StaticTag), CEREAL_NVP(DisabledTag),
					CEREAL_NVP(DeadlyTag), CEREAL_NVP(IdTag));
			}
		};

		struct SerializableEntity {
			uint32_t id = 0;
			std::string name = "Entity";
			SerializableComponents components{};

			template <typename Archive>
			void serialize(Archive& archive) {
				archive(CEREAL_NVP(id), CEREAL_NVP(name), CEREAL_NVP(components));
			}
		};

		struct SerializableProject {
			int projectVersion = 1;
			std::string activeScene;
			std::vector<SerializableEntity> entities;

			template <typename Archive>
			void serialize(Archive& archive) {
				archive(CEREAL_NVP(projectVersion), CEREAL_NVP(activeScene), CEREAL_NVP(entities));
			}
		};

		SerializableVec2 ToSerializable(const Vec2& vec) { return { vec.x, vec.y }; }
		SerializableColor ToSerializable(const Color& color) { return { color.r, color.g, color.b, color.a }; }
		SerializableTextureHandle ToSerializable(const TextureHandle& handle) { return { handle.index, handle.generation }; }

		Vec2 ToRuntime(const SerializableVec2& vec) { return { vec.x, vec.y }; }
		Color ToRuntime(const SerializableColor& color) { return { color.r, color.g, color.b, color.a }; }
		TextureHandle ToRuntime(const SerializableTextureHandle& handle) { return { handle.index, handle.generation }; }
	}

	bool LoadSceneFromFile(Scene& scene, const std::filesystem::path& projectPath, std::string& outError) {
		SerializableProject project;
		if (!Cereal::DeserializeFromJsonFile(projectPath, project, outError)) {
			outError = "Invalid project file format: " + outError;
			return false;
		}

		std::vector<EntityHandle> entitiesToDestroy;
		for (const EntityHandle entityHandle : scene.GetRegistry().view<entt::entity>()) {
			entitiesToDestroy.push_back(entityHandle);
		}
		for (const EntityHandle entityHandle : entitiesToDestroy) {
			scene.DestroyEntity(entityHandle);
		}

		for (const SerializableEntity& serializableEntity : project.entities) {
			Entity entity = scene.CreateEntity(serializableEntity.name.empty() ? "Entity" : serializableEntity.name);
			const SerializableComponents& components = serializableEntity.components;

			if (components.Transform2D) {
				auto& tr = entity.GetComponent<Transform2DComponent>();
				tr.Position = ToRuntime(components.Transform2D->position);
				tr.Scale = ToRuntime(components.Transform2D->scale);
				tr.Rotation = components.Transform2D->rotation;
			}

			if (components.Camera2D) {
				if (!entity.HasComponent<Camera2DComponent>()) {
					entity.AddComponent<Camera2DComponent>();
				}
				auto& cam = entity.GetComponent<Camera2DComponent>();
				cam.SetOrthographicSize(components.Camera2D->orthographicSize);
				cam.SetZoom(components.Camera2D->zoom);
			}

			if (components.SpriteRenderer) {
				if (!entity.HasComponent<SpriteRendererComponent>()) {
					entity.AddComponent<SpriteRendererComponent>();
				}
				auto& sr = entity.GetComponent<SpriteRendererComponent>();
				sr.SortingOrder = components.SpriteRenderer->sortingOrder;
				sr.SortingLayer = static_cast<uint8_t>(components.SpriteRenderer->sortingLayer);
				sr.TextureHandle = ToRuntime(components.SpriteRenderer->textureHandle);
				sr.Color = ToRuntime(components.SpriteRenderer->color);
			}

			if (components.Image) {
				if (!entity.HasComponent<ImageComponent>()) {
					entity.AddComponent<ImageComponent>();
				}
				auto& image = entity.GetComponent<ImageComponent>();
				image.TextureHandle = ToRuntime(components.Image->textureHandle);
				image.Color = ToRuntime(components.Image->color);
			}

			if (components.ParticleSystem2D) {
				if (!entity.HasComponent<ParticleSystem2DComponent>()) {
					entity.AddComponent<ParticleSystem2DComponent>();
				}
				auto& particleSystem = entity.GetComponent<ParticleSystem2DComponent>();
				particleSystem.SetTexture(ToRuntime(components.ParticleSystem2D->textureHandle));
				particleSystem.SetIsEmitting(components.ParticleSystem2D->isEmitting);
				particleSystem.SetIsSimulating(components.ParticleSystem2D->isSimulating);
			}

			if (components.AudioSource) {
				if (!entity.HasComponent<AudioSourceComponent>()) {
					entity.AddComponent<AudioSourceComponent>();
				}
				auto& audio = entity.GetComponent<AudioSourceComponent>();
				audio.SetVolume(components.AudioSource->volume);
				audio.SetPitch(components.AudioSource->pitch);
				audio.SetLoop(components.AudioSource->loop);
				audio.SetAudioHandle(AudioHandle(components.AudioSource->audioHandle));
			}

			if (components.StaticTag && !entity.HasComponent<StaticTag>()) { entity.AddComponent<StaticTag>(); }
			if (components.DisabledTag && !entity.HasComponent<DisabledTag>()) { entity.AddComponent<DisabledTag>(); }
			if (components.DeadlyTag && !entity.HasComponent<DeadlyTag>()) { entity.AddComponent<DeadlyTag>(); }
			if (components.IdTag && !entity.HasComponent<IdTag>()) { entity.AddComponent<IdTag>(); }
		}

		return true;
	}

	bool SaveSceneToFile(const Scene& scene, const std::string& path, std::string& outError) {
		try {
			const std::filesystem::path outputPath = File::NormalizeProjectFilePath(path);
			SerializableProject project;
			project.activeScene = scene.GetName();

			std::vector<EntityHandle> handles;
			for (const EntityHandle handle : scene.GetRegistry().view<entt::entity>()) {
				handles.push_back(handle);
			}
			std::sort(handles.begin(), handles.end(), [](const EntityHandle a, const EntityHandle b) {
				return entt::to_integral(a) < entt::to_integral(b);
				});

			project.entities.reserve(handles.size());
			for (const EntityHandle handle : handles) {
				const Entity entity = scene.GetEntity(handle);
				SerializableEntity serializedEntity;
				serializedEntity.id = entt::to_integral(entity.GetHandle());
				serializedEntity.name = entity.GetName();

				if (entity.HasComponent<Transform2DComponent>()) {
					const auto& transform = entity.GetComponent<Transform2DComponent>();
					serializedEntity.components.Transform2D = SerializableTransform2D{
						ToSerializable(transform.Position),
						ToSerializable(transform.Scale),
						transform.Rotation
					};
				}

				if (entity.HasComponent<Camera2DComponent>()) {
					const auto& camera = entity.GetComponent<Camera2DComponent>();
					serializedEntity.components.Camera2D = SerializableCamera2D{
						camera.GetOrthographicSize(),
						camera.GetZoom()
					};
				}

				if (entity.HasComponent<SpriteRendererComponent>()) {
					const auto& sprite = entity.GetComponent<SpriteRendererComponent>();
					serializedEntity.components.SpriteRenderer = SerializableSpriteRenderer{
						sprite.SortingOrder,
						sprite.SortingLayer,
						ToSerializable(sprite.TextureHandle),
						ToSerializable(sprite.Color)
					};
				}

				if (entity.HasComponent<ImageComponent>()) {
					const auto& image = entity.GetComponent<ImageComponent>();
					serializedEntity.components.Image = SerializableImage{
						ToSerializable(image.TextureHandle),
						ToSerializable(image.Color)
					};
				}

				if (entity.HasComponent<ParticleSystem2DComponent>()) {
					const auto& particles = entity.GetComponent<ParticleSystem2DComponent>();
					serializedEntity.components.ParticleSystem2D = SerializableParticleSystem2D{
						particles.IsEmitting(),
						particles.IsSimulating(),
						ToSerializable(particles.GetTextureHandle())
					};
				}

				if (entity.HasComponent<AudioSourceComponent>()) {
					const auto& audio = entity.GetComponent<AudioSourceComponent>();
					serializedEntity.components.AudioSource = SerializableAudioSource{
						audio.GetVolume(),
						audio.GetPitch(),
						audio.IsLooping(),
						audio.GetAudioHandle().GetHandle()
					};
				}

				serializedEntity.components.StaticTag = entity.HasComponent<StaticTag>();
				serializedEntity.components.DisabledTag = entity.HasComponent<DisabledTag>();
				serializedEntity.components.DeadlyTag = entity.HasComponent<DeadlyTag>();
				serializedEntity.components.IdTag = entity.HasComponent<IdTag>();

				project.entities.push_back(std::move(serializedEntity));
			}

			const std::filesystem::path tempPath = outputPath.string() + ".tmp";
			if (!Cereal::SerializeToJsonFile(tempPath, project, outError)) {
				return false;
			}

			std::error_code ec;
			std::filesystem::rename(tempPath, outputPath, ec);
			if (ec) {
				std::filesystem::remove(outputPath, ec);
				ec.clear();
				std::filesystem::rename(tempPath, outputPath, ec);
			}

			if (ec) {
				outError = "Failed to finalize save file: " + ec.message();
				return false;
			}

			return true;
		}
		catch (const std::exception& e) {
			outError = e.what();
			return false;
		}
	}
}
#include "pch.hpp"
#include "Editor/EditorSceneSerializer.hpp"

#include "Core/Application.hpp"
#include "Scene/Scene.hpp"
#include "Components/Components.hpp"
#include "Utils/File.hpp"


#include <algorithm>
#include <cstdlib>
#include <fstream>

namespace Bolt {
	namespace {
		std::string Escape(const std::string& input) {
			std::string escaped;
			escaped.reserve(input.size());

			for (char c : input) {
				switch (c) {
				case '\\': escaped += "\\\\"; break;
				case '"': escaped += "\\\""; break;
				case '\n': escaped += "\\n"; break;
				case '\r': escaped += "\\r"; break;
				case '\t': escaped += "\\t"; break;
				default: escaped += c; break;
				}
			}
			return escaped;
		}


		void WriteColor(std::ofstream& out, const Color& color) {
			out << "{ \"r\": " << color.r << ", \"g\": " << color.g << ", \"b\": " << color.b << ", \"a\": " << color.a << " }";
		}
	}

	EditorSceneSaveResult EditorSceneSerializer::Save(const Scene& scene) {
		EditorSceneSaveResult result;

		const std::filesystem::path projectRoot = File::GetRuntimeProjectDirectory(Application::GetName());
		const std::filesystem::path scenePath = projectRoot / "SampleScene.scene";

		std::error_code ec;
		std::filesystem::create_directories(projectRoot, ec);
		if (ec) {
			result.Message = "Failed to create project directory: " + projectRoot.string();
			return result;
		}

		std::ofstream file(scenePath, std::ios::trunc);
		if (!file.is_open()) {
			result.Message = "Failed to open scene file for writing: " + scenePath.string();
			return result;
		}

		auto view = scene.GetRegistry().view<entt::entity>();
		file << "{\n";
		file << "  \"sceneName\": \"" << Escape(scene.GetName()) << "\",\n";
		file << "  \"entityCount\": " << view.size() << ",\n";
		file << "  \"entities\": [\n";

		size_t entityIndex = 0;
		for (const EntityHandle handle : view) {
			const Entity entity = scene.GetEntity(handle);

			file << "    {\n";
			file << "      \"handle\": " << static_cast<uint32_t>(handle) << ",\n";
			file << "      \"components\": {\n";

			bool wroteComponent = false;
			auto writeSeparator = [&]() {
				if (wroteComponent) {
					file << ",\n";
				}
				wroteComponent = true;
				};

			if (entity.HasComponent<NameComponent>()) {
				writeSeparator();
				file << "        \"NameComponent\": { \"name\": \"" << Escape(entity.GetComponent<NameComponent>().Name) << "\" }";
			}
			if (entity.HasComponent<Transform2DComponent>()) {
				const auto& tr = entity.GetComponent<Transform2DComponent>();
				writeSeparator();
				file << "        \"Transform2DComponent\": { "
					<< "\"position\": { \"x\": " << tr.Position.x << ", \"y\": " << tr.Position.y << " }, "
					<< "\"scale\": { \"x\": " << tr.Scale.x << ", \"y\": " << tr.Scale.y << " }, "
					<< "\"rotation\": " << tr.Rotation
					<< " }";
			}
			if (entity.HasComponent<RectTransformComponent>()) {
				const auto& rt = entity.GetComponent<RectTransformComponent>();
				writeSeparator();
				file << "        \"RectTransformComponent\": { "
					<< "\"position\": { \"x\": " << rt.Position.x << ", \"y\": " << rt.Position.y << " }, "
					<< "\"pivot\": { \"x\": " << rt.Pivot.x << ", \"y\": " << rt.Pivot.y << " }, "
					<< "\"width\": " << rt.Width << ", "
					<< "\"height\": " << rt.Height << ", "
					<< "\"rotation\": " << rt.Rotation << ", "
					<< "\"scale\": { \"x\": " << rt.Scale.x << ", \"y\": " << rt.Scale.y << " }"
					<< " }";
			}
			if (entity.HasComponent<Camera2DComponent>()) {
				const auto& cam = entity.GetComponent<Camera2DComponent>();
				writeSeparator();
				file << "        \"Camera2DComponent\": { "
					<< "\"orthographicSize\": " << cam.GetOrthographicSize() << ", "
					<< "\"zoom\": " << cam.GetZoom() << " }";
			}
			if (entity.HasComponent<SpriteRendererComponent>()) {
				const auto& renderer = entity.GetComponent<SpriteRendererComponent>();
				writeSeparator();
				file << "        \"SpriteRendererComponent\": { "
					<< "\"sortingOrder\": " << renderer.SortingOrder << ", "
					<< "\"sortingLayer\": " << static_cast<uint32_t>(renderer.SortingLayer) << ", "
					<< "\"textureHandle\": { "
					<< "\"index\": " << renderer.TextureHandle.index << ", "
					<< "\"generation\": " << renderer.TextureHandle.generation << " }, "
					<< "\"color\": ";
				WriteColor(file, renderer.Color);
				file << " }";
			}
			if (entity.HasComponent<ImageComponent>()) {
				const auto& image = entity.GetComponent<ImageComponent>();
				writeSeparator();
				file << "        \"ImageComponent\": { "
					<< "\"textureHandle\": { "
					<< "\"index\": " << image.TextureHandle.index << ", "
					<< "\"generation\": " << image.TextureHandle.generation << " }, "
					<< "\"color\": ";
				WriteColor(file, image.Color);
				file << " }";
			}
			if (entity.HasComponent<AudioSourceComponent>()) {
				const auto& audioSource = entity.GetComponent<AudioSourceComponent>();
				writeSeparator();
				file << "        \"AudioSourceComponent\": { "
					<< "\"audioHandle\": " << audioSource.GetAudioHandle().GetHandle() << ", "
					<< "\"volume\": " << audioSource.GetVolume() << ", "
					<< "\"pitch\": " << audioSource.GetPitch() << ", "
					<< "\"loop\": " << (audioSource.IsLooping() ? "true" : "false") << " }";
			}
			if (entity.HasComponent<IdTag>()) {
				writeSeparator();
				file << "        \"IdTag\": true";
			}
			if (entity.HasComponent<StaticTag>()) {
				writeSeparator();
				file << "        \"StaticTag\": true";
			}
			if (entity.HasComponent<DisabledTag>()) {
				writeSeparator();
				file << "        \"DisabledTag\": true";
			}
			if (entity.HasComponent<DeadlyTag>()) {
				writeSeparator();
				file << "        \"DeadlyTag\": true";
			}

			file << "\n      }\n";
			file << "    }";
			if (++entityIndex < view.size()) {
				file << ",";
			}
			file << "\n";
		}

		file << "  ]\n";
		file << "}\n";

		if (!file.good()) {
			result.Message = "Failed while writing scene file: " + scenePath.string();
			return result;
		}

		result.Succeeded = true;
		result.FilePath = scenePath;
		result.Message = "Scene saved: " + scenePath.string();
		return result;
	}
}
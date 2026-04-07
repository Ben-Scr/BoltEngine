#include "pch.hpp"
#include "Serialization/SceneSerializer.hpp"
#include "Serialization/File.hpp"
#include "Scene/Scene.hpp"
#include "Scene/EntityHelper.hpp"
#include "Components/General/NameComponent.hpp"
#include "Components/General/Transform2DComponent.hpp"
#include "Components/Graphics/SpriteRendererComponent.hpp"
#include "Components/Graphics/Camera2DComponent.hpp"
#include "Scripting/ScriptComponent.hpp"
#include "Core/Log.hpp"

#include <sstream>
#include <fstream>
#include <filesystem>

namespace Bolt {

	static constexpr int SCENE_FORMAT_VERSION = 1;

	// ── JSON Writing Helpers ────────────────────────────────────────

	static std::string Escape(const std::string& s) {
		std::string out;
		for (char c : s) {
			if (c == '\\') out += "\\\\";
			else if (c == '"') out += "\\\"";
			else if (c == '\n') out += "\\n";
			else out += c;
		}
		return out;
	}

	// ── JSON Reading Helpers ────────────────────────────────────────

	// Find matching closing brace for an opening brace at position pos
	static size_t FindMatchingBrace(const std::string& s, size_t openPos) {
		int depth = 0;
		for (size_t i = openPos; i < s.size(); i++) {
			if (s[i] == '{') depth++;
			else if (s[i] == '}') { depth--; if (depth == 0) return i; }
		}
		return std::string::npos;
	}

	// Extract a string value: "key": "value"
	static std::string ReadString(const std::string& json, const std::string& key, const std::string& def = "") {
		std::string search = "\"" + key + "\"";
		auto pos = json.find(search);
		if (pos == std::string::npos) return def;

		pos = json.find(':', pos + search.size());
		if (pos == std::string::npos) return def;

		pos = json.find('"', pos + 1);
		if (pos == std::string::npos) return def;

		auto end = json.find('"', pos + 1);
		if (end == std::string::npos) return def;

		return json.substr(pos + 1, end - pos - 1);
	}

	// Extract a numeric value: "key": 1.5
	static float ReadFloat(const std::string& json, const std::string& key, float def = 0.0f) {
		std::string search = "\"" + key + "\"";
		auto pos = json.find(search);
		if (pos == std::string::npos) return def;

		pos = json.find(':', pos + search.size());
		if (pos == std::string::npos) return def;
		pos++;

		while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
		if (pos >= json.size()) return def;

		auto end = pos;
		while (end < json.size() && json[end] != ',' && json[end] != '}' && json[end] != '\n' && json[end] != ' ')
			end++;

		std::string val = json.substr(pos, end - pos);
		try { return std::stof(val); }
		catch (...) { return def; }
	}

	static int ReadInt(const std::string& json, const std::string& key, int def = 0) {
		return static_cast<int>(ReadFloat(json, key, static_cast<float>(def)));
	}

	// Extract a nested object block: "Key": { ... } → returns the { ... } content
	static std::string ReadBlock(const std::string& json, const std::string& key) {
		std::string search = "\"" + key + "\"";
		auto pos = json.find(search);
		if (pos == std::string::npos) return "";

		auto braceStart = json.find('{', pos + search.size());
		if (braceStart == std::string::npos) return "";

		auto braceEnd = FindMatchingBrace(json, braceStart);
		if (braceEnd == std::string::npos) return "";

		return json.substr(braceStart, braceEnd - braceStart + 1);
	}

	// Extract a string array: "Key": ["a", "b"] → returns vector
	static std::vector<std::string> ReadStringArray(const std::string& json, const std::string& key) {
		std::vector<std::string> result;
		std::string search = "\"" + key + "\"";
		auto pos = json.find(search);
		if (pos == std::string::npos) return result;

		auto arrStart = json.find('[', pos);
		auto arrEnd = json.find(']', arrStart);
		if (arrStart == std::string::npos || arrEnd == std::string::npos) return result;

		std::string arr = json.substr(arrStart + 1, arrEnd - arrStart - 1);
		size_t p = 0;
		while ((p = arr.find('"', p)) != std::string::npos) {
			auto end = arr.find('"', p + 1);
			if (end == std::string::npos) break;
			result.push_back(arr.substr(p + 1, end - p - 1));
			p = end + 1;
		}
		return result;
	}

	// ── SaveToFile ──────────────────────────────────────────────────

	bool SceneSerializer::SaveToFile(Scene& scene, const std::string& path) {
		try {
			auto parentDir = std::filesystem::path(path).parent_path();
			if (!std::filesystem::exists(parentDir))
				std::filesystem::create_directories(parentDir);

			std::stringstream ss;
			ss << "{\n";
			ss << "  \"version\": " << SCENE_FORMAT_VERSION << ",\n";
			ss << "  \"name\": \"" << Escape(scene.GetName()) << "\",\n";
			ss << "  \"entities\": [\n";

			auto& registry = scene.GetRegistry();
			auto view = registry.view<entt::entity>();
			bool first = true;

			for (auto entity : view) {
				if (!first) ss << ",\n";
				first = false;

				std::string name = "Entity";
				if (registry.all_of<NameComponent>(entity))
					name = registry.get<NameComponent>(entity).Name;

				ss << "    {\n";
				ss << "      \"name\": \"" << Escape(name) << "\"";

				if (registry.all_of<Transform2DComponent>(entity)) {
					auto& t = registry.get<Transform2DComponent>(entity);
					ss << ",\n      \"Transform2D\": {"
					   << " \"posX\": " << t.Position.x
					   << ", \"posY\": " << t.Position.y
					   << ", \"rotation\": " << t.Rotation
					   << ", \"scaleX\": " << t.Scale.x
					   << ", \"scaleY\": " << t.Scale.y << " }";
				}

				if (registry.all_of<SpriteRendererComponent>(entity)) {
					auto& s = registry.get<SpriteRendererComponent>(entity);
					ss << ",\n      \"SpriteRenderer\": {"
					   << " \"r\": " << s.Color.r
					   << ", \"g\": " << s.Color.g
					   << ", \"b\": " << s.Color.b
					   << ", \"a\": " << s.Color.a
					   << ", \"sortOrder\": " << s.SortingOrder << " }";
				}

				if (registry.all_of<Camera2DComponent>(entity)) {
					auto& c = registry.get<Camera2DComponent>(entity);
					ss << ",\n      \"Camera2D\": {"
					   << " \"orthoSize\": " << c.GetOrthographicSize()
					   << ", \"zoom\": " << c.GetZoom() << " }";
				}

				if (registry.all_of<ScriptComponent>(entity)) {
					auto& sc = registry.get<ScriptComponent>(entity);
					if (!sc.Scripts.empty()) {
						ss << ",\n      \"Scripts\": [";
						for (size_t i = 0; i < sc.Scripts.size(); i++) {
							if (i > 0) ss << ", ";
							ss << "\"" << Escape(sc.Scripts[i].GetClassName()) << "\"";
						}
						ss << "]";
					}
				}

				ss << "\n    }";
			}

			ss << "\n  ]\n}\n";
			File::WriteAllText(path, ss.str());
			BT_CORE_INFO_TAG("SceneSerializer", "Saved scene: {}", scene.GetName());
			return true;
		}
		catch (const std::exception& e) {
			BT_CORE_ERROR_TAG("SceneSerializer", "Save failed: {}", e.what());
			return false;
		}
	}

	// ── LoadFromFile ────────────────────────────────────────────────

	bool SceneSerializer::LoadFromFile(Scene& scene, const std::string& path) {
		try {
			if (!File::Exists(path)) {
				BT_CORE_WARN_TAG("SceneSerializer", "Scene file not found: {}", path);
				return false;
			}

			std::ifstream in(path);
			std::string json((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
			in.close();

			if (json.empty()) {
				BT_CORE_WARN_TAG("SceneSerializer", "Scene file is empty: {}", path);
				return false;
			}

			// Read version for future compatibility
			int version = ReadInt(json, "version", 1);
			if (version > SCENE_FORMAT_VERSION) {
				BT_CORE_WARN_TAG("SceneSerializer", "Scene version {} is newer than supported ({})",
					version, SCENE_FORMAT_VERSION);
			}

			// Clear existing entities
			scene.GetRegistry().clear();

			// Find entities array
			auto entitiesPos = json.find("\"entities\"");
			if (entitiesPos == std::string::npos) {
				BT_CORE_WARN_TAG("SceneSerializer", "No entities array in scene file");
				return true; // Valid empty scene
			}

			auto arrayStart = json.find('[', entitiesPos);
			if (arrayStart == std::string::npos) return true;

			// Find each entity object by tracking brace depth
			int braceDepth = 0;
			size_t entityStart = std::string::npos;

			for (size_t i = arrayStart + 1; i < json.size(); i++) {
				if (json[i] == '{') {
					if (braceDepth == 0) entityStart = i;
					braceDepth++;
				}
				else if (json[i] == '}') {
					braceDepth--;
					if (braceDepth == 0 && entityStart != std::string::npos) {
						std::string entityJson = json.substr(entityStart, i - entityStart + 1);
						DeserializeEntity(scene, entityJson);
						entityStart = std::string::npos;
					}
				}
				else if (json[i] == ']' && braceDepth == 0) {
					break;
				}
			}

			// Ensure scene has at least one camera
			auto camView = scene.GetRegistry().view<Camera2DComponent>();
			if (camView.size() == 0) {
				EntityHelper::CreateCamera2DEntity();
				BT_CORE_INFO_TAG("SceneSerializer", "Added default camera (none in scene file)");
			}

			BT_CORE_INFO_TAG("SceneSerializer", "Loaded scene: {}", scene.GetName());
			return true;
		}
		catch (const std::exception& e) {
			BT_CORE_ERROR_TAG("SceneSerializer", "Load failed: {}", e.what());
			return false;
		}
	}

	void SceneSerializer::DeserializeEntity(Scene& scene, const std::string& entityJson) {
		std::string name = ReadString(entityJson, "name", "Entity");
		EntityHandle entity = scene.CreateEntity(name).GetHandle();

		// Transform2D (always exists from CreateEntity, overwrite with saved values)
		std::string t2dBlock = ReadBlock(entityJson, "Transform2D");
		if (!t2dBlock.empty()) {
			auto& t = scene.GetComponent<Transform2DComponent>(entity);
			t.Position.x = ReadFloat(t2dBlock, "posX", 0.0f);
			t.Position.y = ReadFloat(t2dBlock, "posY", 0.0f);
			t.Rotation = ReadFloat(t2dBlock, "rotation", 0.0f);
			t.Scale.x = ReadFloat(t2dBlock, "scaleX", 1.0f);
			t.Scale.y = ReadFloat(t2dBlock, "scaleY", 1.0f);
		}

		// SpriteRenderer
		std::string srBlock = ReadBlock(entityJson, "SpriteRenderer");
		if (!srBlock.empty()) {
			auto& s = scene.AddComponent<SpriteRendererComponent>(entity);
			s.Color.r = ReadFloat(srBlock, "r", 1.0f);
			s.Color.g = ReadFloat(srBlock, "g", 1.0f);
			s.Color.b = ReadFloat(srBlock, "b", 1.0f);
			s.Color.a = ReadFloat(srBlock, "a", 1.0f);
			s.SortingOrder = static_cast<short>(ReadInt(srBlock, "sortOrder", 0));
		}

		// Camera2D
		std::string camBlock = ReadBlock(entityJson, "Camera2D");
		if (!camBlock.empty()) {
			auto& c = scene.AddComponent<Camera2DComponent>(entity);
			c.SetOrthographicSize(ReadFloat(camBlock, "orthoSize", 5.0f));
			c.SetZoom(ReadFloat(camBlock, "zoom", 1.0f));
		}

		// Scripts
		auto scripts = ReadStringArray(entityJson, "Scripts");
		if (!scripts.empty()) {
			auto& sc = scene.AddComponent<ScriptComponent>(entity);
			for (auto& className : scripts)
				sc.AddScript(className);
		}
	}

} // namespace Bolt

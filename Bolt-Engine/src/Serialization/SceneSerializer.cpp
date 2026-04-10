#include "pch.hpp"
#include "Serialization/SceneSerializer.hpp"
#include "Serialization/File.hpp"
#include "Scene/Scene.hpp"
#include "Scene/EntityHelper.hpp"
#include "Components/General/NameComponent.hpp"
#include "Components/General/Transform2DComponent.hpp"
#include "Components/Graphics/SpriteRendererComponent.hpp"
#include "Components/Graphics/Camera2DComponent.hpp"
#include "Components/Physics/Rigidbody2DComponent.hpp"
#include "Components/Physics/BoxCollider2DComponent.hpp"
#include "Components/Audio/AudioSourceComponent.hpp"
#include "Components/General/RectTransformComponent.hpp"
#include "Components/Graphics/ImageComponent.hpp"
#include "Components/Graphics/ParticleSystem2DComponent.hpp"
#include "Components/General/UUIDComponent.hpp"
#include "Components/Tags.hpp"
#include "Scripting/ScriptComponent.hpp"
#include "Components/Physics/BoltBody2DComponent.hpp"
#include "Components/Physics/BoltBoxCollider2DComponent.hpp"
#include "Components/Physics/BoltCircleCollider2DComponent.hpp"
#include "Graphics/TextureManager.hpp"
#include "Audio/AudioManager.hpp"
#include "Physics/PhysicsTypes.hpp"
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

	static std::string Unescape(const std::string& s) {
		std::string out;
		for (size_t i = 0; i < s.size(); i++) {
			if (s[i] == '\\' && i + 1 < s.size()) {
				char next = s[i + 1];
				if (next == '\\') { out += '\\'; i++; }
				else if (next == '"') { out += '"'; i++; }
				else if (next == 'n') { out += '\n'; i++; }
				else out += s[i];
			} else {
				out += s[i];
			}
		}
		return out;
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

		return Unescape(json.substr(pos + 1, end - pos - 1));
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

	static uint64_t ReadUint64(const std::string& json, const std::string& key, uint64_t def = 0) {
		std::string search = "\"" + key + "\"";
		auto pos = json.find(search);
		if (pos == std::string::npos) return def;
		pos = json.find(':', pos + search.size());
		if (pos == std::string::npos) return def;
		pos++;
		while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
		try { return std::stoull(json.substr(pos)); } catch (...) { return def; }
	}

	static bool ReadBool(const std::string& json, const std::string& key, bool def = false) {
		std::string search = "\"" + key + "\"";
		auto pos = json.find(search);
		if (pos == std::string::npos) return def;
		pos = json.find(':', pos + search.size());
		if (pos == std::string::npos) return def;
		auto rest = json.substr(pos + 1, 20);
		if (rest.find("true") < rest.find_first_of(",}")) return true;
		if (rest.find("false") < rest.find_first_of(",}")) return false;
		return def;
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

			// Collect entities into a stable vector so the file order
			// matches the view order. On reload the same view order is
			// reproduced because CreateEntity appends to the registry
			// in file order, and the view iterates in that same order.
			std::vector<entt::entity> entities(view.begin(), view.end());

			bool first = true;

			for (auto entity : entities) {
				if (!first) ss << ",\n";
				first = false;

				std::string name = "Entity";
				if (registry.all_of<NameComponent>(entity))
					name = registry.get<NameComponent>(entity).Name;

				ss << "    {\n";
				ss << "      \"name\": \"" << Escape(name) << "\"";

				if (registry.all_of<UUIDComponent>(entity)) {
					uint64_t uuid = static_cast<uint64_t>(registry.get<UUIDComponent>(entity).Id);
					ss << ",\n      \"uuid\": " << uuid;
				}

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
					std::string texName = TextureManager::GetTextureName(s.TextureHandle);
					ss << ",\n      \"SpriteRenderer\": {"
					   << " \"r\": " << s.Color.r
					   << ", \"g\": " << s.Color.g
					   << ", \"b\": " << s.Color.b
					   << ", \"a\": " << s.Color.a
					   << ", \"sortOrder\": " << s.SortingOrder
					   << ", \"sortLayer\": " << static_cast<int>(s.SortingLayer);
					if (!texName.empty()) {
					   ss << ", \"texture\": \"" << Escape(texName) << "\"";
					   Texture2D* tex = TextureManager::GetTexture(s.TextureHandle);
					   if (tex) {
					       ss << ", \"filter\": " << static_cast<int>(tex->GetFilter())
					          << ", \"wrapU\": " << static_cast<int>(tex->GetWrapU())
					          << ", \"wrapV\": " << static_cast<int>(tex->GetWrapV());
					   }
					}
					ss << " }";
				}

				if (registry.all_of<Rigidbody2DComponent>(entity)) {
					auto& rb = registry.get<Rigidbody2DComponent>(entity);
					ss << ",\n      \"Rigidbody2D\": {"
					   << " \"bodyType\": " << static_cast<int>(rb.GetBodyType())
					   << ", \"gravityScale\": " << rb.GetGravityScale()
					   << ", \"mass\": " << rb.GetMass()
					   << " }";
				}

				if (registry.all_of<BoxCollider2DComponent>(entity)) {
					auto& bc = registry.get<BoxCollider2DComponent>(entity);
					Vec2 scale = bc.GetScale();
					Vec2 center = bc.GetCenter();
					ss << ",\n      \"BoxCollider2D\": {"
					   << " \"scaleX\": " << scale.x
					   << ", \"scaleY\": " << scale.y
					   << ", \"centerX\": " << center.x
					   << ", \"centerY\": " << center.y
					   << " }";
				}

				if (registry.all_of<AudioSourceComponent>(entity)) {
					auto& audio = registry.get<AudioSourceComponent>(entity);
					ss << ",\n      \"AudioSource\": {"
					   << " \"volume\": " << audio.GetVolume()
					   << ", \"pitch\": " << audio.GetPitch()
					   << ", \"loop\": " << (audio.IsLooping() ? "true" : "false")
					   << " }";
				}

				// Tags
				if (registry.all_of<StaticTag>(entity))
					ss << ",\n      \"static\": true";
				if (registry.all_of<DisabledTag>(entity))
					ss << ",\n      \"disabled\": true";
				if (registry.all_of<DeadlyTag>(entity))
					ss << ",\n      \"deadly\": true";

				if (registry.all_of<Camera2DComponent>(entity)) {
					auto& c = registry.get<Camera2DComponent>(entity);
					ss << ",\n      \"Camera2D\": {"
					   << " \"orthoSize\": " << c.GetOrthographicSize()
					   << ", \"zoom\": " << c.GetZoom()
					   << ", \"clearR\": " << c.GetClearColor().r
					   << ", \"clearG\": " << c.GetClearColor().g
					   << ", \"clearB\": " << c.GetClearColor().b
					   << ", \"clearA\": " << c.GetClearColor().a
					   << " }";
				}

				// Bolt-Physics components
				if (registry.all_of<BoltBody2DComponent>(entity)) {
					auto& b = registry.get<BoltBody2DComponent>(entity);
					ss << ",\n      \"BoltBody2D\": {"
					   << " \"type\": " << static_cast<int>(b.Type)
					   << ", \"mass\": " << b.Mass
					   << ", \"useGravity\": " << (b.UseGravity ? "true" : "false")
					   << ", \"boundaryCheck\": " << (b.BoundaryCheck ? "true" : "false")
					   << " }";
				}
				if (registry.all_of<BoltBoxCollider2DComponent>(entity)) {
					auto& c = registry.get<BoltBoxCollider2DComponent>(entity);
					ss << ",\n      \"BoltBoxCollider2D\": {"
					   << " \"halfX\": " << c.HalfExtents.x
					   << ", \"halfY\": " << c.HalfExtents.y
					   << " }";
				}
				if (registry.all_of<BoltCircleCollider2DComponent>(entity)) {
					auto& c = registry.get<BoltCircleCollider2DComponent>(entity);
					ss << ",\n      \"BoltCircleCollider2D\": { \"radius\": " << c.Radius << " }";
				}

				if (registry.all_of<ParticleSystem2DComponent>(entity)) {
					auto& ps = registry.get<ParticleSystem2DComponent>(entity);
					int shapeType = std::holds_alternative<ParticleSystem2DComponent::CircleParams>(ps.Shape) ? 0 : 1;
					ss << ",\n      \"ParticleSystem2D\": {"
					   << " \"playOnAwake\": " << (ps.PlayOnAwake ? "true" : "false")
					   << ", \"lifetime\": " << ps.ParticleSettings.LifeTime
					   << ", \"speed\": " << ps.ParticleSettings.Speed
					   << ", \"scale\": " << ps.ParticleSettings.Scale
					   << ", \"gravityX\": " << ps.ParticleSettings.Gravity.x
					   << ", \"gravityY\": " << ps.ParticleSettings.Gravity.y
					   << ", \"useGravity\": " << (ps.ParticleSettings.UseGravity ? "true" : "false")
					   << ", \"useRandomColors\": " << (ps.ParticleSettings.UseRandomColors ? "true" : "false")
					   << ", \"moveDirectionX\": " << ps.ParticleSettings.MoveDirection.x
					   << ", \"moveDirectionY\": " << ps.ParticleSettings.MoveDirection.y
					   << ", \"emitOverTime\": " << ps.EmissionSettings.EmitOverTime
					   << ", \"rateOverDistance\": " << ps.EmissionSettings.RateOverDistance
					   << ", \"emissionSpace\": " << static_cast<int>(ps.EmissionSettings.EmissionSpace)
					   << ", \"shapeType\": " << shapeType;
					if (shapeType == 0) {
						auto& circle = std::get<ParticleSystem2DComponent::CircleParams>(ps.Shape);
						ss << ", \"radius\": " << circle.Radius
						   << ", \"isOnCircle\": " << (circle.IsOnCircle ? "true" : "false");
					} else {
						auto& square = std::get<ParticleSystem2DComponent::SquareParams>(ps.Shape);
						ss << ", \"halfExtendsX\": " << square.HalfExtends.x
						   << ", \"halfExtendsY\": " << square.HalfExtends.y;
					}
					ss << ", \"maxParticles\": " << ps.RenderingSettings.MaxParticles
					   << ", \"colorR\": " << ps.RenderingSettings.Color.r
					   << ", \"colorG\": " << ps.RenderingSettings.Color.g
					   << ", \"colorB\": " << ps.RenderingSettings.Color.b
					   << ", \"colorA\": " << ps.RenderingSettings.Color.a
					   << ", \"sortOrder\": " << ps.RenderingSettings.SortingOrder
					   << ", \"sortLayer\": " << static_cast<int>(ps.RenderingSettings.SortingLayer);
					std::string psTexName = TextureManager::GetTextureName(ps.GetTextureHandle());
					if (!psTexName.empty()) {
						ss << ", \"texture\": \"" << Escape(psTexName) << "\"";
					}
					ss << " }";
				}

				if (registry.all_of<RectTransformComponent>(entity)) {
					auto& rt = registry.get<RectTransformComponent>(entity);
					ss << ",\n      \"RectTransform\": {"
					   << " \"posX\": " << rt.Position.x
					   << ", \"posY\": " << rt.Position.y
					   << ", \"pivotX\": " << rt.Pivot.x
					   << ", \"pivotY\": " << rt.Pivot.y
					   << ", \"width\": " << rt.Width
					   << ", \"height\": " << rt.Height
					   << ", \"rotation\": " << rt.Rotation
					   << ", \"scaleX\": " << rt.Scale.x
					   << ", \"scaleY\": " << rt.Scale.y
					   << " }";
				}

				if (registry.all_of<ImageComponent>(entity)) {
					auto& img = registry.get<ImageComponent>(entity);
					ss << ",\n      \"Image\": {"
					   << " \"r\": " << img.Color.r
					   << ", \"g\": " << img.Color.g
					   << ", \"b\": " << img.Color.b
					   << ", \"a\": " << img.Color.a;
					std::string imgTexName = TextureManager::GetTextureName(img.TextureHandle);
					if (!imgTexName.empty()) {
						ss << ", \"texture\": \"" << Escape(imgTexName) << "\"";
					}
					ss << " }";
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
			scene.ClearDirty();
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

			// Update scene name from file path
			scene.SetName(std::filesystem::path(path).stem().string());

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

			scene.ClearDirty();
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

		// UUID (overwrite the auto-generated one if saved value exists)
		uint64_t savedUUID = ReadUint64(entityJson, "uuid", 0);
		if (savedUUID != 0 && scene.HasComponent<UUIDComponent>(entity)) {
			scene.GetComponent<UUIDComponent>(entity).Id = UUID(savedUUID);
		}

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
			s.SortingLayer = static_cast<uint8_t>(ReadInt(srBlock, "sortLayer", 0));
			std::string texPath = ReadString(srBlock, "texture");
			if (!texPath.empty()) {
				s.TextureHandle = TextureManager::LoadTexture(texPath);
				// Restore texture filter and wrap settings
				Texture2D* tex = TextureManager::GetTexture(s.TextureHandle);
				if (tex) {
					int filter = ReadInt(srBlock, "filter", static_cast<int>(Filter::Point));
					int wrapU = ReadInt(srBlock, "wrapU", static_cast<int>(Wrap::Clamp));
					int wrapV = ReadInt(srBlock, "wrapV", static_cast<int>(Wrap::Clamp));
					tex->SetSampler(static_cast<Filter>(filter), static_cast<Wrap>(wrapU), static_cast<Wrap>(wrapV));
				}
			}
		}

		// Rigidbody2D (Box2D)
		std::string rbBlock = ReadBlock(entityJson, "Rigidbody2D");
		if (!rbBlock.empty()) {
			auto& rb = scene.AddComponent<Rigidbody2DComponent>(entity);
			int bodyType = ReadInt(rbBlock, "bodyType", static_cast<int>(BodyType::Dynamic));
			rb.SetBodyType(static_cast<BodyType>(bodyType));
			rb.SetGravityScale(ReadFloat(rbBlock, "gravityScale", 1.0f));
			rb.SetMass(ReadFloat(rbBlock, "mass", 1.0f));
		}

		// BoxCollider2D (Box2D)
		std::string bcBlock = ReadBlock(entityJson, "BoxCollider2D");
		if (!bcBlock.empty()) {
			scene.AddComponent<BoxCollider2DComponent>(entity);
			// Scale and center are set via the construct hook from Transform
		}

		// AudioSource
		std::string audioBlock = ReadBlock(entityJson, "AudioSource");
		if (!audioBlock.empty()) {
			auto& audio = scene.AddComponent<AudioSourceComponent>(entity);
			audio.SetVolume(ReadFloat(audioBlock, "volume", 1.0f));
			audio.SetPitch(ReadFloat(audioBlock, "pitch", 1.0f));
			audio.SetLoop(ReadBool(audioBlock, "loop", false));
		}

		// Tags
		if (ReadBool(entityJson, "static", false))
			scene.AddComponent<StaticTag>(entity);
		if (ReadBool(entityJson, "disabled", false))
			scene.AddComponent<DisabledTag>(entity);
		if (ReadBool(entityJson, "deadly", false))
			scene.AddComponent<DeadlyTag>(entity);

		// Camera2D
		std::string camBlock = ReadBlock(entityJson, "Camera2D");
		if (!camBlock.empty()) {
			auto& c = scene.AddComponent<Camera2DComponent>(entity);
			c.SetOrthographicSize(ReadFloat(camBlock, "orthoSize", 5.0f));
			c.SetZoom(ReadFloat(camBlock, "zoom", 1.0f));
			c.SetClearColor(Color(
				ReadFloat(camBlock, "clearR", 0.1f),
				ReadFloat(camBlock, "clearG", 0.1f),
				ReadFloat(camBlock, "clearB", 0.1f),
				ReadFloat(camBlock, "clearA", 1.0f)));
		}

		// Bolt-Physics components
		// Note: on_construct hooks create runtime objects with default values.
		// After setting the deserialized values on the struct, we must sync them
		// to the actual BoltPhys::Body / BoltPhys::Collider objects.
		std::string boltBodyBlock = ReadBlock(entityJson, "BoltBody2D");
		if (!boltBodyBlock.empty()) {
			auto& b = scene.AddComponent<BoltBody2DComponent>(entity);
			b.Type = static_cast<BoltPhys::BodyType>(ReadInt(boltBodyBlock, "type", 1));
			b.Mass = ReadFloat(boltBodyBlock, "mass", 1.0f);
			b.UseGravity = ReadBool(boltBodyBlock, "useGravity", true);
			b.BoundaryCheck = ReadBool(boltBodyBlock, "boundaryCheck", false);
			// Sync deserialized values to the runtime Body
			if (b.m_Body) {
				b.m_Body->SetBodyType(b.Type);
				b.m_Body->SetMass(b.Mass);
				b.m_Body->SetGravityEnabled(b.UseGravity);
				b.m_Body->SetBoundaryCheckEnabled(b.BoundaryCheck);
			}
		}
		std::string boltBoxBlock = ReadBlock(entityJson, "BoltBoxCollider2D");
		if (!boltBoxBlock.empty()) {
			auto& c = scene.AddComponent<BoltBoxCollider2DComponent>(entity);
			c.HalfExtents = { ReadFloat(boltBoxBlock, "halfX", 0.5f), ReadFloat(boltBoxBlock, "halfY", 0.5f) };
			if (c.m_Collider)
				c.m_Collider->SetHalfExtents({ c.HalfExtents.x, c.HalfExtents.y });
		}
		std::string boltCircleBlock = ReadBlock(entityJson, "BoltCircleCollider2D");
		if (!boltCircleBlock.empty()) {
			auto& c = scene.AddComponent<BoltCircleCollider2DComponent>(entity);
			c.Radius = ReadFloat(boltCircleBlock, "radius", 0.5f);
			if (c.m_Collider)
				c.m_Collider->SetRadius(c.Radius);
		}

		// ParticleSystem2D
		std::string psBlock = ReadBlock(entityJson, "ParticleSystem2D");
		if (!psBlock.empty()) {
			auto& ps = scene.AddComponent<ParticleSystem2DComponent>(entity);
			ps.PlayOnAwake = ReadBool(psBlock, "playOnAwake", true);
			ps.ParticleSettings.LifeTime = ReadFloat(psBlock, "lifetime", 1.0f);
			ps.ParticleSettings.Speed = ReadFloat(psBlock, "speed", 5.0f);
			ps.ParticleSettings.Scale = ReadFloat(psBlock, "scale", 1.0f);
			ps.ParticleSettings.Gravity.x = ReadFloat(psBlock, "gravityX", 0.0f);
			ps.ParticleSettings.Gravity.y = ReadFloat(psBlock, "gravityY", 0.0f);
			ps.ParticleSettings.UseGravity = ReadBool(psBlock, "useGravity", false);
			ps.ParticleSettings.UseRandomColors = ReadBool(psBlock, "useRandomColors", false);
			ps.ParticleSettings.MoveDirection.x = ReadFloat(psBlock, "moveDirectionX", 0.0f);
			ps.ParticleSettings.MoveDirection.y = ReadFloat(psBlock, "moveDirectionY", 0.0f);
			ps.EmissionSettings.EmitOverTime = static_cast<uint16_t>(ReadInt(psBlock, "emitOverTime", 10));
			ps.EmissionSettings.RateOverDistance = static_cast<uint16_t>(ReadInt(psBlock, "rateOverDistance", 0));
			ps.EmissionSettings.EmissionSpace = static_cast<ParticleSystem2DComponent::Space>(ReadInt(psBlock, "emissionSpace", 1));
			int shapeType = ReadInt(psBlock, "shapeType", 0);
			if (shapeType == 0) {
				ParticleSystem2DComponent::CircleParams circle;
				circle.Radius = ReadFloat(psBlock, "radius", 1.0f);
				circle.IsOnCircle = ReadBool(psBlock, "isOnCircle", false);
				ps.Shape = circle;
			} else {
				ParticleSystem2DComponent::SquareParams square;
				square.HalfExtends.x = ReadFloat(psBlock, "halfExtendsX", 1.0f);
				square.HalfExtends.y = ReadFloat(psBlock, "halfExtendsY", 1.0f);
				ps.Shape = square;
			}
			ps.RenderingSettings.MaxParticles = static_cast<uint32_t>(ReadInt(psBlock, "maxParticles", 1000));
			ps.RenderingSettings.Color.r = ReadFloat(psBlock, "colorR", 1.0f);
			ps.RenderingSettings.Color.g = ReadFloat(psBlock, "colorG", 1.0f);
			ps.RenderingSettings.Color.b = ReadFloat(psBlock, "colorB", 1.0f);
			ps.RenderingSettings.Color.a = ReadFloat(psBlock, "colorA", 1.0f);
			ps.RenderingSettings.SortingOrder = static_cast<short>(ReadInt(psBlock, "sortOrder", 0));
			ps.RenderingSettings.SortingLayer = static_cast<uint8_t>(ReadInt(psBlock, "sortLayer", 0));
			std::string psTexPath = ReadString(psBlock, "texture");
			if (!psTexPath.empty()) {
				ps.SetTexture(TextureManager::LoadTexture(psTexPath));
			}
		}

		// RectTransform
		std::string rtBlock = ReadBlock(entityJson, "RectTransform");
		if (!rtBlock.empty()) {
			auto& rt = scene.AddComponent<RectTransformComponent>(entity);
			rt.Position.x = ReadFloat(rtBlock, "posX", 0.0f);
			rt.Position.y = ReadFloat(rtBlock, "posY", 0.0f);
			rt.Pivot.x = ReadFloat(rtBlock, "pivotX", 0.0f);
			rt.Pivot.y = ReadFloat(rtBlock, "pivotY", 0.0f);
			rt.Width = ReadFloat(rtBlock, "width", 100.0f);
			rt.Height = ReadFloat(rtBlock, "height", 100.0f);
			rt.Rotation = ReadFloat(rtBlock, "rotation", 0.0f);
			rt.Scale.x = ReadFloat(rtBlock, "scaleX", 1.0f);
			rt.Scale.y = ReadFloat(rtBlock, "scaleY", 1.0f);
		}

		// Image
		std::string imgBlock = ReadBlock(entityJson, "Image");
		if (!imgBlock.empty()) {
			auto& img = scene.AddComponent<ImageComponent>(entity);
			img.Color.r = ReadFloat(imgBlock, "r", 1.0f);
			img.Color.g = ReadFloat(imgBlock, "g", 1.0f);
			img.Color.b = ReadFloat(imgBlock, "b", 1.0f);
			img.Color.a = ReadFloat(imgBlock, "a", 1.0f);
			std::string imgTexPath = ReadString(imgBlock, "texture");
			if (!imgTexPath.empty()) {
				img.TextureHandle = TextureManager::LoadTexture(imgTexPath);
			}
		}

		// Scripts
		auto scripts = ReadStringArray(entityJson, "Scripts");
		if (!scripts.empty()) {
			auto& sc = scene.AddComponent<ScriptComponent>(entity);
			for (auto& className : scripts)
				sc.AddScript(className);
		}
	}

	// ── Prefab: SaveEntityToFile ─────────────────────────────────────

	bool SceneSerializer::SaveEntityToFile(Scene& scene, EntityHandle entity, const std::string& path) {
		try {
			auto parentDir = std::filesystem::path(path).parent_path();
			if (!std::filesystem::exists(parentDir))
				std::filesystem::create_directories(parentDir);

			auto& registry = scene.GetRegistry();

			std::string name = "Entity";
			if (registry.all_of<NameComponent>(entity))
				name = registry.get<NameComponent>(entity).Name;

			std::stringstream ss;
			ss << "{\n  \"prefab\": {\n";
			ss << "      \"name\": \"" << Escape(name) << "\"";

			if (registry.all_of<UUIDComponent>(entity)) {
				uint64_t uuid = static_cast<uint64_t>(registry.get<UUIDComponent>(entity).Id);
				ss << ",\n      \"uuid\": " << uuid;
			}

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
				std::string texName = TextureManager::GetTextureName(s.TextureHandle);
				ss << ",\n      \"SpriteRenderer\": {"
				   << " \"r\": " << s.Color.r
				   << ", \"g\": " << s.Color.g
				   << ", \"b\": " << s.Color.b
				   << ", \"a\": " << s.Color.a
				   << ", \"sortOrder\": " << s.SortingOrder
				   << ", \"sortLayer\": " << static_cast<int>(s.SortingLayer);
				if (!texName.empty()) {
				   ss << ", \"texture\": \"" << Escape(texName) << "\"";
				   Texture2D* tex = TextureManager::GetTexture(s.TextureHandle);
				   if (tex) {
				       ss << ", \"filter\": " << static_cast<int>(tex->GetFilter())
				          << ", \"wrapU\": " << static_cast<int>(tex->GetWrapU())
				          << ", \"wrapV\": " << static_cast<int>(tex->GetWrapV());
				   }
				}
				ss << " }";
			}

			if (registry.all_of<Rigidbody2DComponent>(entity)) {
				auto& rb = registry.get<Rigidbody2DComponent>(entity);
				ss << ",\n      \"Rigidbody2D\": {"
				   << " \"bodyType\": " << static_cast<int>(rb.GetBodyType())
				   << ", \"gravityScale\": " << rb.GetGravityScale()
				   << ", \"mass\": " << rb.GetMass()
				   << " }";
			}

			if (registry.all_of<BoxCollider2DComponent>(entity)) {
				auto& bc = registry.get<BoxCollider2DComponent>(entity);
				Vec2 scale = bc.GetScale();
				Vec2 center = bc.GetCenter();
				ss << ",\n      \"BoxCollider2D\": {"
				   << " \"scaleX\": " << scale.x
				   << ", \"scaleY\": " << scale.y
				   << ", \"centerX\": " << center.x
				   << ", \"centerY\": " << center.y
				   << " }";
			}

			if (registry.all_of<AudioSourceComponent>(entity)) {
				auto& audio = registry.get<AudioSourceComponent>(entity);
				ss << ",\n      \"AudioSource\": {"
				   << " \"volume\": " << audio.GetVolume()
				   << ", \"pitch\": " << audio.GetPitch()
				   << ", \"loop\": " << (audio.IsLooping() ? "true" : "false")
				   << " }";
			}

			if (registry.all_of<StaticTag>(entity))
				ss << ",\n      \"static\": true";
			if (registry.all_of<DisabledTag>(entity))
				ss << ",\n      \"disabled\": true";
			if (registry.all_of<DeadlyTag>(entity))
				ss << ",\n      \"deadly\": true";

			if (registry.all_of<Camera2DComponent>(entity)) {
				auto& c = registry.get<Camera2DComponent>(entity);
				ss << ",\n      \"Camera2D\": {"
				   << " \"orthoSize\": " << c.GetOrthographicSize()
				   << ", \"zoom\": " << c.GetZoom()
				   << ", \"clearR\": " << c.GetClearColor().r
				   << ", \"clearG\": " << c.GetClearColor().g
				   << ", \"clearB\": " << c.GetClearColor().b
				   << ", \"clearA\": " << c.GetClearColor().a
				   << " }";
			}

			if (registry.all_of<BoltBody2DComponent>(entity)) {
				auto& b = registry.get<BoltBody2DComponent>(entity);
				ss << ",\n      \"BoltBody2D\": {"
				   << " \"type\": " << static_cast<int>(b.Type)
				   << ", \"mass\": " << b.Mass
				   << ", \"useGravity\": " << (b.UseGravity ? "true" : "false")
				   << ", \"boundaryCheck\": " << (b.BoundaryCheck ? "true" : "false")
				   << " }";
			}
			if (registry.all_of<BoltBoxCollider2DComponent>(entity)) {
				auto& c = registry.get<BoltBoxCollider2DComponent>(entity);
				ss << ",\n      \"BoltBoxCollider2D\": {"
				   << " \"halfX\": " << c.HalfExtents.x
				   << ", \"halfY\": " << c.HalfExtents.y
				   << " }";
			}
			if (registry.all_of<BoltCircleCollider2DComponent>(entity)) {
				auto& c = registry.get<BoltCircleCollider2DComponent>(entity);
				ss << ",\n      \"BoltCircleCollider2D\": { \"radius\": " << c.Radius << " }";
			}

			if (registry.all_of<ParticleSystem2DComponent>(entity)) {
				auto& ps = registry.get<ParticleSystem2DComponent>(entity);
				int shapeType = std::holds_alternative<ParticleSystem2DComponent::CircleParams>(ps.Shape) ? 0 : 1;
				ss << ",\n      \"ParticleSystem2D\": {"
				   << " \"playOnAwake\": " << (ps.PlayOnAwake ? "true" : "false")
				   << ", \"lifetime\": " << ps.ParticleSettings.LifeTime
				   << ", \"speed\": " << ps.ParticleSettings.Speed
				   << ", \"scale\": " << ps.ParticleSettings.Scale
				   << ", \"gravityX\": " << ps.ParticleSettings.Gravity.x
				   << ", \"gravityY\": " << ps.ParticleSettings.Gravity.y
				   << ", \"useGravity\": " << (ps.ParticleSettings.UseGravity ? "true" : "false")
				   << ", \"useRandomColors\": " << (ps.ParticleSettings.UseRandomColors ? "true" : "false")
				   << ", \"moveDirectionX\": " << ps.ParticleSettings.MoveDirection.x
				   << ", \"moveDirectionY\": " << ps.ParticleSettings.MoveDirection.y
				   << ", \"emitOverTime\": " << ps.EmissionSettings.EmitOverTime
				   << ", \"rateOverDistance\": " << ps.EmissionSettings.RateOverDistance
				   << ", \"emissionSpace\": " << static_cast<int>(ps.EmissionSettings.EmissionSpace)
				   << ", \"shapeType\": " << shapeType;
				if (shapeType == 0) {
					auto& circle = std::get<ParticleSystem2DComponent::CircleParams>(ps.Shape);
					ss << ", \"radius\": " << circle.Radius
					   << ", \"isOnCircle\": " << (circle.IsOnCircle ? "true" : "false");
				} else {
					auto& square = std::get<ParticleSystem2DComponent::SquareParams>(ps.Shape);
					ss << ", \"halfExtendsX\": " << square.HalfExtends.x
					   << ", \"halfExtendsY\": " << square.HalfExtends.y;
				}
				ss << ", \"maxParticles\": " << ps.RenderingSettings.MaxParticles
				   << ", \"colorR\": " << ps.RenderingSettings.Color.r
				   << ", \"colorG\": " << ps.RenderingSettings.Color.g
				   << ", \"colorB\": " << ps.RenderingSettings.Color.b
				   << ", \"colorA\": " << ps.RenderingSettings.Color.a
				   << ", \"sortOrder\": " << ps.RenderingSettings.SortingOrder
				   << ", \"sortLayer\": " << static_cast<int>(ps.RenderingSettings.SortingLayer);
				std::string psTexName = TextureManager::GetTextureName(ps.GetTextureHandle());
				if (!psTexName.empty()) {
					ss << ", \"texture\": \"" << Escape(psTexName) << "\"";
				}
				ss << " }";
			}

			if (registry.all_of<RectTransformComponent>(entity)) {
				auto& rt = registry.get<RectTransformComponent>(entity);
				ss << ",\n      \"RectTransform\": {"
				   << " \"posX\": " << rt.Position.x
				   << ", \"posY\": " << rt.Position.y
				   << ", \"pivotX\": " << rt.Pivot.x
				   << ", \"pivotY\": " << rt.Pivot.y
				   << ", \"width\": " << rt.Width
				   << ", \"height\": " << rt.Height
				   << ", \"rotation\": " << rt.Rotation
				   << ", \"scaleX\": " << rt.Scale.x
				   << ", \"scaleY\": " << rt.Scale.y
				   << " }";
			}

			if (registry.all_of<ImageComponent>(entity)) {
				auto& img = registry.get<ImageComponent>(entity);
				ss << ",\n      \"Image\": {"
				   << " \"r\": " << img.Color.r
				   << ", \"g\": " << img.Color.g
				   << ", \"b\": " << img.Color.b
				   << ", \"a\": " << img.Color.a;
				std::string imgTexName = TextureManager::GetTextureName(img.TextureHandle);
				if (!imgTexName.empty()) {
					ss << ", \"texture\": \"" << Escape(imgTexName) << "\"";
				}
				ss << " }";
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

			ss << "\n  }\n}\n";
			File::WriteAllText(path, ss.str());
			BT_CORE_INFO_TAG("SceneSerializer", "Saved prefab: {}", name);
			return true;
		}
		catch (const std::exception& e) {
			BT_CORE_ERROR_TAG("SceneSerializer", "SaveEntityToFile failed: {}", e.what());
			return false;
		}
	}

	// ── Prefab: LoadEntityFromFile ───────────────────────────────────

	EntityHandle SceneSerializer::LoadEntityFromFile(Scene& scene, const std::string& path) {
		try {
			if (!File::Exists(path)) {
				BT_CORE_WARN_TAG("SceneSerializer", "Prefab file not found: {}", path);
				return entt::null;
			}

			std::ifstream in(path);
			std::string json((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
			in.close();

			std::string entityBlock = ReadBlock(json, "prefab");
			if (entityBlock.empty()) {
				BT_CORE_WARN_TAG("SceneSerializer", "No prefab block in file: {}", path);
				return entt::null;
			}

			// Remember entity count before deserialization
			auto countBefore = scene.GetRegistry().view<entt::entity>().size();

			DeserializeEntity(scene, entityBlock);

			// Find the newly created entity (last in registry)
			auto view = scene.GetRegistry().view<entt::entity>();
			if (view.size() > countBefore) {
				// The newest entity is the one just created
				auto it = view.begin();
				return *it; // entt views iterate newest-first
			}

			return entt::null;
		}
		catch (const std::exception& e) {
			BT_CORE_ERROR_TAG("SceneSerializer", "LoadEntityFromFile failed: {}", e.what());
			return entt::null;
		}
	}

} // namespace Bolt

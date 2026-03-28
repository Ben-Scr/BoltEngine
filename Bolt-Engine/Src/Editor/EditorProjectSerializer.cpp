#include <pch.hpp>

#include "EditorProjectSerializer.hpp"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <variant>

#include "Components/Components.hpp"

namespace Bolt {
	namespace {
		std::string EscapeJsonString(std::string_view input) {
			std::string escaped;
			escaped.reserve(input.size() + 8);
			for (const unsigned char ch : input) {
				switch (ch) {
				case '\\': escaped += "\\\\"; break;
				case '"': escaped += "\\\""; break;
				case '\n': escaped += "\\n"; break;
				case '\r': escaped += "\\r"; break;
				case '\t': escaped += "\\t"; break;
				default:
					if (ch < 0x20) {
						char buffer[7];
						std::snprintf(buffer, sizeof(buffer), "\\u%04X", ch);
						escaped += buffer;
					}
					else {
						escaped.push_back(static_cast<char>(ch));
					}
					break;
				}
			}
			return escaped;
		}

		void WriteVec2(std::ostringstream& out, const Vec2& vec) {
			out << "{\"x\":" << vec.x << ",\"y\":" << vec.y << "}";
		}

		void WriteColor(std::ostringstream& out, const Color& color) {
			out << "{\"r\":" << color.r << ",\"g\":" << color.g << ",\"b\":" << color.b << ",\"a\":" << color.a << "}";
		}

		struct JsonValue {
			using Object = std::unordered_map<std::string, JsonValue>;
			using Array = std::vector<JsonValue>;
			using Variant = std::variant<std::nullptr_t, bool, double, std::string, Object, Array>;
			Variant Data = nullptr;

			bool IsObject() const { return std::holds_alternative<Object>(Data); }
			bool IsArray() const { return std::holds_alternative<Array>(Data); }
			bool IsString() const { return std::holds_alternative<std::string>(Data); }
			bool IsNumber() const { return std::holds_alternative<double>(Data); }
			bool IsBool() const { return std::holds_alternative<bool>(Data); }

			const Object* AsObject() const { return std::get_if<Object>(&Data); }
			const Array* AsArray() const { return std::get_if<Array>(&Data); }
			const std::string* AsString() const { return std::get_if<std::string>(&Data); }
			const double* AsNumber() const { return std::get_if<double>(&Data); }
			const bool* AsBool() const { return std::get_if<bool>(&Data); }
		};

		class JsonParser {
		public:
			explicit JsonParser(std::string content)
				: m_Content(std::move(content)) {}

			bool Parse(JsonValue& out, std::string& outError) {
				SkipWhitespace();
				if (!ParseValue(out, outError)) {
					return false;
				}

				SkipWhitespace();
				if (!IsEof()) {
					outError = "Unexpected characters at end of file.";
					return false;
				}
				return true;
			}

		private:
			static bool IsHexDigit(char c) {
				return (c >= '0' && c <= '9') ||
					(c >= 'a' && c <= 'f') ||
					(c >= 'A' && c <= 'F');
			}

			static uint32_t HexValue(char c) {
				if (c >= '0' && c <= '9') {
					return static_cast<uint32_t>(c - '0');
				}
				if (c >= 'a' && c <= 'f') {
					return static_cast<uint32_t>(10 + (c - 'a'));
				}
				return static_cast<uint32_t>(10 + (c - 'A'));
			}

			static void AppendCodePointUtf8(std::string& out, uint32_t codePoint) {
				if (codePoint <= 0x7F) {
					out.push_back(static_cast<char>(codePoint));
					return;
				}
				if (codePoint <= 0x7FF) {
					out.push_back(static_cast<char>(0xC0 | (codePoint >> 6)));
					out.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
					return;
				}
				if (codePoint <= 0xFFFF) {
					out.push_back(static_cast<char>(0xE0 | (codePoint >> 12)));
					out.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
					out.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
					return;
				}
				out.push_back(static_cast<char>(0xF0 | (codePoint >> 18)));
				out.push_back(static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F)));
				out.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
				out.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
			}

			bool ParseUnicodeEscape(uint32_t& outCodePoint, std::string& outError) {
				if (m_Position + 4 > m_Content.size()) {
					outError = "Invalid unicode escape sequence.";
					return false;
				}

				uint32_t value = 0;
				for (int i = 0; i < 4; ++i) {
					const char hex = m_Content[m_Position++];
					if (!IsHexDigit(hex)) {
						outError = "Invalid unicode escape sequence.";
						return false;
					}
					value = (value << 4) | HexValue(hex);
				}

				outCodePoint = value;
				return true;
			}

			bool ParseValue(JsonValue& out, std::string& outError) {
				SkipWhitespace();
				if (IsEof()) {
					outError = "Unexpected end of file.";
					return false;
				}

				switch (Peek()) {
				case '{': return ParseObject(out, outError);
				case '[': return ParseArray(out, outError);
				case '"': {
					std::string parsedString;
					if (!ParseString(parsedString, outError)) {
						return false;
					}
					out.Data = std::move(parsedString);
					return true;
				}
				case 't': return ParseLiteral("true", true, out, outError);
				case 'f': return ParseLiteral("false", false, out, outError);
				case 'n': return ParseNull(out, outError);
				default: return ParseNumber(out, outError);
				}
			}

			bool ParseObject(JsonValue& out, std::string& outError) {
				if (!TryConsume('{')) {
					outError = "Expected '{' to start object.";
					return false;
				}
				JsonValue::Object object;
				SkipWhitespace();
				if (TryConsume('}')) {
					out.Data = std::move(object);
					return true;
				}

				while (!IsEof()) {
					std::string key;
					if (!ParseString(key, outError)) {
						return false;
					}

					SkipWhitespace();
					if (!TryConsume(':')) {
						outError = "Expected ':' after object key.";
						return false;
					}

					JsonValue value;
					if (!ParseValue(value, outError)) {
						return false;
					}
					object.emplace(std::move(key), std::move(value));

					SkipWhitespace();
					if (TryConsume('}')) {
						out.Data = std::move(object);
						return true;
					}

					if (!TryConsume(',')) {
						outError = "Expected ',' or '}' in object.";
						return false;
					}

					SkipWhitespace();
				}

				outError = "Unexpected end of object.";
				return false;
			}

			bool ParseArray(JsonValue& out, std::string& outError) {
				if (!TryConsume('[')) {
					outError = "Expected '[' to start array.";
					return false;
				}

				JsonValue::Array array;
				SkipWhitespace();
				if (TryConsume(']')) {
					out.Data = std::move(array);
					return true;
				}

				while (!IsEof()) {
					JsonValue value;
					if (!ParseValue(value, outError)) {
						return false;
					}
					array.push_back(std::move(value));

					SkipWhitespace();
					if (TryConsume(']')) {
						out.Data = std::move(array);
						return true;
					}

					if (!TryConsume(',')) {
						outError = "Expected ',' or ']' in array.";
						return false;
					}

					SkipWhitespace();
				}

				outError = "Unexpected end of array.";
				return false;
			}

			bool ParseString(std::string& out, std::string& outError) {
				if (!TryConsume('"')) {
					outError = "Expected string.";
					return false;
				}

				out.clear();
				while (!IsEof()) {
					const char c = Get();
					if (c == '"') {
						return true;
					}
					if (c == '\\') {
						if (IsEof()) {
							outError = "Invalid escape sequence.";
							return false;
						}

						const char escaped = Get();
						switch (escaped) {
						case '"': out.push_back('"'); break;
						case '\\': out.push_back('\\'); break;
						case '/': out.push_back('/'); break;
						case 'b': out.push_back('\b'); break;
						case 'f': out.push_back('\f'); break;
						case 'n': out.push_back('\n'); break;
						case 'r': out.push_back('\r'); break;
						case 't': out.push_back('\t'); break;
						default:
							outError = "Unsupported escape sequence.";
							return false;
						}
						continue;
					}
					out.push_back(c);
				}

				outError = "Unterminated string.";
				return false;
			}

			bool ParseLiteral(const char* literal, bool value, JsonValue& out, std::string& outError) {
				const size_t length = std::strlen(literal);
				if (m_Content.compare(m_Position, length, literal) != 0) {
					outError = "Invalid token in JSON document.";
					return false;
				}
				m_Position += length;
				out.Data = value;
				return true;
			}

			bool ParseNull(JsonValue& out, std::string& outError) {
				if (m_Content.compare(m_Position, 4, "null") != 0) {
					outError = "Invalid token in JSON document.";
					return false;
				}
				m_Position += 4;
				out.Data = nullptr;
				return true;
			}

			bool ParseNumber(JsonValue& out, std::string& outError) {
				const size_t start = m_Position;

				if (Peek() == '-') {
					++m_Position;
				}
				while (!IsEof() && std::isdigit(static_cast<unsigned char>(Peek()))) {
					++m_Position;
				}
				if (!IsEof() && Peek() == '.') {
					++m_Position;
					while (!IsEof() && std::isdigit(static_cast<unsigned char>(Peek()))) {
						++m_Position;
					}
				}
				if (!IsEof() && (Peek() == 'e' || Peek() == 'E')) {
					++m_Position;
					if (!IsEof() && (Peek() == '+' || Peek() == '-')) {
						++m_Position;
					}
					while (!IsEof() && std::isdigit(static_cast<unsigned char>(Peek()))) {
						++m_Position;
					}
				}

				const std::string token = m_Content.substr(start, m_Position - start);
				char* end = nullptr;
				const double value = std::strtod(token.c_str(), &end);
				if (token.empty() || end == token.c_str() || *end != '\0') {
					outError = "Invalid number: '" + token + "'";
					return false;
				}

				out.Data = value;
				return true;
			}

			void SkipWhitespace() {
				while (!IsEof() && std::isspace(static_cast<unsigned char>(m_Content[m_Position]))) {
					++m_Position;
				}
			}

			bool IsEof() const { return m_Position >= m_Content.size(); }
			char Peek() const { return m_Content[m_Position]; }
			char Get() { return m_Content[m_Position++]; }
			
			bool TryConsume(char ch) {
				if (!IsEof() && Peek() == ch) {
					++m_Position;
					return true;
				}
				return false;
			}

			std::string m_Content;
			size_t m_Position = 0;
		};

		const JsonValue* TryGetObjectValue(const JsonValue::Object& object, const std::string& key) {
			const auto it = object.find(key);
			return it == object.end() ? nullptr : &it->second;
		}

		std::optional<float> GetFloatValue(const JsonValue::Object& object, const std::string& key) {
			const JsonValue* value = TryGetObjectValue(object, key);
			if (!value || !value->IsNumber()) {
				return std::nullopt;
			}
			return static_cast<float>(*value->AsNumber());
		}

		std::optional<int> GetIntValue(const JsonValue::Object& object, const std::string& key) {
			const JsonValue* value = TryGetObjectValue(object, key);
			if (!value || !value->IsNumber()) {
				return std::nullopt;
			}
			const double numberValue = *value->AsNumber();
			if (!std::isfinite(numberValue)) {
				return std::nullopt;
			}
			if (std::floor(numberValue) != numberValue) {
				return std::nullopt;
			}
			if (numberValue < static_cast<double>(std::numeric_limits<int>::min()) ||
				numberValue > static_cast<double>(std::numeric_limits<int>::max())) {
				return std::nullopt;
			}
			return static_cast<int>(numberValue);
		}

		std::optional<bool> GetBoolValue(const JsonValue::Object& object, const std::string& key) {
			const JsonValue* value = TryGetObjectValue(object, key);
			if (!value || !value->IsBool()) {
				return std::nullopt;
			}
			return *value->AsBool();
		}

		bool TryReadVec2(const JsonValue* value, Vec2& out) {
			if (!value || !value->IsObject()) {
				return false;
			}
			const auto& object = *value->AsObject();
			const auto x = GetFloatValue(object, "x");
			const auto y = GetFloatValue(object, "y");
			if (!x.has_value() || !y.has_value()) {
				return false;
			}
			out.x = *x;
			out.y = *y;
			return true;
		}

		bool TryReadColor(const JsonValue* value, Color& out) {
			if (!value || !value->IsObject()) {
				return false;
			}
			const auto& object = *value->AsObject();
			const auto r = GetFloatValue(object, "r");
			const auto g = GetFloatValue(object, "g");
			const auto b = GetFloatValue(object, "b");
			const auto a = GetFloatValue(object, "a");
			if (!r || !g || !b || !a) {
				return false;
			}
			out.r = *r;
			out.g = *g;
			out.b = *b;
			out.a = *a;
			return true;
		}
	}

	std::optional<uint16_t> ToUInt16(const std::optional<int>& value) {
		if (!value || *value < 0 || *value > static_cast<int>(std::numeric_limits<uint16_t>::max())) {
			return std::nullopt;
		}
		return static_cast<uint16_t>(*value);
	}

	std::optional<uint8_t> ToUInt8(const std::optional<int>& value) {
		if (!value || *value < 0 || *value > static_cast<int>(std::numeric_limits<uint8_t>::max())) {
			return std::nullopt;
		}
		return static_cast<uint8_t>(*value);
	}

	std::optional<short> ToShort(const std::optional<int>& value) {
		if (!value ||
			*value < static_cast<int>(std::numeric_limits<short>::min()) ||
			*value > static_cast<int>(std::numeric_limits<short>::max())) {
			return std::nullopt;
		}
		return static_cast<short>(*value);
	}

	std::optional<TextureHandle> ReadTextureHandle(const JsonValue::Object& object, const std::string& key) {
		const JsonValue* textureValue = TryGetObjectValue(object, key);
		if (!textureValue || !textureValue->IsObject()) {
			return std::nullopt;
		}

		const JsonValue::Object& textureObject = *textureValue->AsObject();
		const auto index = ToUInt16(GetIntValue(textureObject, "index"));
		const auto generation = ToUInt16(GetIntValue(textureObject, "generation"));
		if (!index || !generation) {
			return std::nullopt;
		}

		return TextureHandle(*index, *generation);
	}

	bool LoadSceneFromFile(Scene& scene, const std::filesystem::path& projectPath, std::string& outError) {
		std::ifstream input(projectPath);
		if (!input.is_open()) {
			outError = "Unable to open project file: " + projectPath.string();
			return false;
		}

		std::stringstream buffer;
		buffer << input.rdbuf();
		const std::string fileContent = buffer.str();

		JsonValue root;
		JsonParser parser(fileContent);
		if (!parser.Parse(root, outError)) {
			outError = "Invalid project file format: " + outError;
			return false;
		}

		const JsonValue::Object* rootObject = root.AsObject();
		if (!rootObject) {
			outError = "Project root must be a JSON object.";
			return false;
		}

		const JsonValue* entitiesValue = TryGetObjectValue(*rootObject, "entities");
		if (!entitiesValue || !entitiesValue->IsArray()) {
			outError = "Project file has no valid 'entities' array.";
			return false;
		}

		std::vector<EntityHandle> entitiesToDestroy;
		for (const EntityHandle entityHandle : scene.GetRegistry().view<entt::entity>()) {
			entitiesToDestroy.push_back(entityHandle);
		}
		for (const EntityHandle entityHandle : entitiesToDestroy) {
			scene.DestroyEntity(entityHandle);
		}

		const JsonValue::Array& entitiesArray = *entitiesValue->AsArray();
		for (const JsonValue& entityValue : entitiesArray) {
			const JsonValue::Object* entityObject = entityValue.AsObject();
			if (!entityObject) {
				continue;
			}

			std::string entityName = "Entity";
			if (const JsonValue* nameValue = TryGetObjectValue(*entityObject, "name")) {
				if (const std::string* name = nameValue->AsString()) {
					entityName = *name;
				}
			}

			Entity entity = scene.CreateEntity(entityName);
			const JsonValue* componentsValue = TryGetObjectValue(*entityObject, "components");
			const JsonValue::Object* components = componentsValue ? componentsValue->AsObject() : nullptr;
			if (!components) {
				continue;
			}

			if (const JsonValue* transformValue = TryGetObjectValue(*components, "Transform2D")) {
				if (const JsonValue::Object* transform = transformValue->AsObject()) {
					auto& tr = entity.GetComponent<Transform2DComponent>();
					TryReadVec2(TryGetObjectValue(*transform, "position"), tr.Position);
					TryReadVec2(TryGetObjectValue(*transform, "scale"), tr.Scale);
					if (const auto rotation = GetFloatValue(*transform, "rotation")) {
						tr.Rotation = *rotation;
					}
				}
			}

			if (const JsonValue* cameraValue = TryGetObjectValue(*components, "Camera2D")) {
				if (const JsonValue::Object* camera = cameraValue->AsObject()) {
					if (!entity.HasComponent<Camera2DComponent>()) {
						entity.AddComponent<Camera2DComponent>();
					}
					auto& cam = entity.GetComponent<Camera2DComponent>();
					if (const auto orthoSize = GetFloatValue(*camera, "orthographicSize")) {
						cam.SetOrthographicSize(*orthoSize);
					}
					if (const auto zoom = GetFloatValue(*camera, "zoom")) {
						cam.SetZoom(*zoom);
					}
				}
			}

			if (const JsonValue* spriteValue = TryGetObjectValue(*components, "SpriteRenderer")) {
				if (const JsonValue::Object* sprite = spriteValue->AsObject()) {
					if (!entity.HasComponent<SpriteRendererComponent>()) {
						entity.AddComponent<SpriteRendererComponent>();
					}
					auto& sr = entity.GetComponent<SpriteRendererComponent>();
					if (const auto sortingOrder = ToShort(GetIntValue(*sprite, "sortingOrder"))) {
						sr.SortingOrder = *sortingOrder;
					}
					if (const auto sortingLayer = ToUInt8(GetIntValue(*sprite, "sortingLayer"))) {
						sr.SortingLayer = *sortingLayer;
					}
					if (const auto textureHandle = ReadTextureHandle(*sprite, "textureHandle")) {
						sr.TextureHandle = *textureHandle;
					}
					TryReadColor(TryGetObjectValue(*sprite, "color"), sr.Color);
				}
			}

			if (const JsonValue* imageValue = TryGetObjectValue(*components, "Image")) {
				if (const JsonValue::Object* image = imageValue->AsObject()) {
					if (!entity.HasComponent<ImageComponent>()) {
						entity.AddComponent<ImageComponent>();
					}
					auto& img = entity.GetComponent<ImageComponent>();
					if (const auto textureHandle = ReadTextureHandle(*image, "textureHandle")) {
						img.TextureHandle = *textureHandle;
					}
					TryReadColor(TryGetObjectValue(*image, "color"), img.Color);
				}
			}

			if (const JsonValue* particlesValue = TryGetObjectValue(*components, "ParticleSystem2D")) {
				if (const JsonValue::Object* particles = particlesValue->AsObject()) {
					if (!entity.HasComponent<ParticleSystem2DComponent>()) {
						entity.AddComponent<ParticleSystem2DComponent>();
					}
					auto& particleSystem = entity.GetComponent<ParticleSystem2DComponent>();
					if (const auto textureHandle = ReadTextureHandle(*particles, "textureHandle")) {
						particleSystem.SetTexture(*textureHandle);
					}
					if (const auto isEmitting = GetBoolValue(*particles, "isEmitting")) {
						particleSystem.SetIsEmitting(*isEmitting);
					}
					if (const auto isSimulating = GetBoolValue(*particles, "isSimulating")) {
						particleSystem.SetIsSimulating(*isSimulating);
					}
				}
			}

			if (const auto isStatic = GetBoolValue(*components, "StaticTag"); isStatic && *isStatic && !entity.HasComponent<StaticTag>()) {
				entity.AddComponent<StaticTag>();
			}
			if (const auto isDisabled = GetBoolValue(*components, "DisabledTag"); isDisabled && *isDisabled && !entity.HasComponent<DisabledTag>()) {
				entity.AddComponent<DisabledTag>();
			}
			if (const auto isDeadly = GetBoolValue(*components, "DeadlyTag"); isDeadly && *isDeadly && !entity.HasComponent<DeadlyTag>()) {
				entity.AddComponent<DeadlyTag>();
			}
			if (const auto hasIdTag = GetBoolValue(*components, "IdTag"); hasIdTag && *hasIdTag && !entity.HasComponent<IdTag>()) {
				entity.AddComponent<IdTag>();
			}
		}

		return true;
	}

	bool SaveSceneToFile(const Scene& scene, const std::string& path, std::string& outError) {
		try {
			std::filesystem::path outputPath(path);
			if (!outputPath.has_extension()) {
				outputPath.replace_extension(".boltproject.json");
			}

			const std::filesystem::path parent = outputPath.parent_path();
			if (!parent.empty()) {
				std::filesystem::create_directories(parent);
			}

			std::ostringstream projectContent;
			projectContent << "{\n";
			projectContent << "  \"projectVersion\": 1,\n";
			projectContent << "  \"activeScene\": \"" << EscapeJsonString(scene.GetName()) << "\",\n";
			projectContent << "  \"entities\": [\n";

			std::vector<EntityHandle> handles;
			for (const auto entity : scene.GetRegistry().view<entt::entity>()) {
				handles.push_back(entity);
			}
			std::sort(handles.begin(), handles.end(), [](EntityHandle a, EntityHandle b) {
				return entt::to_integral(a) < entt::to_integral(b);
				});

			for (size_t i = 0; i < handles.size(); ++i) {
				const Entity entity = scene.GetEntity(handles[i]);
				projectContent << "    {\n";
				projectContent << "      \"id\": " << entt::to_integral(entity.GetHandle()) << ",\n";
				projectContent << "      \"name\": \"" << EscapeJsonString(entity.GetName()) << "\",\n";
				projectContent << "      \"components\": {\n";

				bool wroteComponent = false;
				auto writeSeparator = [&]() {
					if (wroteComponent) {
						projectContent << ",\n";
					}
					wroteComponent = true;
					};

				if (entity.HasComponent<Transform2DComponent>()) {
					const auto& transform = entity.GetComponent<Transform2DComponent>();
					writeSeparator();
					projectContent << "        \"Transform2D\": {\"position\":";
					WriteVec2(projectContent, transform.Position);
					projectContent << ",\"scale\":";
					WriteVec2(projectContent, transform.Scale);
					projectContent << ",\"rotation\":" << transform.Rotation << "}";
				}

				if (entity.HasComponent<Camera2DComponent>()) {
					const auto& camera = entity.GetComponent<Camera2DComponent>();
					writeSeparator();
					projectContent << "        \"Camera2D\": {\"orthographicSize\":"
						<< camera.GetOrthographicSize() << ",\"zoom\":" << camera.GetZoom() << "}";
				}

				if (entity.HasComponent<SpriteRendererComponent>()) {
					const auto& sprite = entity.GetComponent<SpriteRendererComponent>();
					writeSeparator();
					projectContent << "        \"SpriteRenderer\": {\"sortingOrder\":" << sprite.SortingOrder
						<< ",\"sortingLayer\":" << static_cast<int>(sprite.SortingLayer)
						<< ",\"textureHandle\":{\"index\":" << sprite.TextureHandle.index
						<< ",\"generation\":" << sprite.TextureHandle.generation << "},\"color\":";
					WriteColor(projectContent, sprite.Color);
					projectContent << "}";
				}

				if (entity.HasComponent<ImageComponent>()) {
					const auto& image = entity.GetComponent<ImageComponent>();
					writeSeparator();
					projectContent << "        \"Image\": {\"textureHandle\":{\"index\":" << image.TextureHandle.index
						<< ",\"generation\":" << image.TextureHandle.generation << "},\"color\":";
					WriteColor(projectContent, image.Color);
					projectContent << "}";
				}

				if (entity.HasComponent<ParticleSystem2DComponent>()) {
					const auto& particles = entity.GetComponent<ParticleSystem2DComponent>();
					writeSeparator();
					projectContent << "        \"ParticleSystem2D\": {\"isEmitting\":"
						<< (particles.IsEmitting() ? "true" : "false")
						<< ",\"isSimulating\":" << (particles.IsSimulating() ? "true" : "false")
						<< ",\"textureHandle\":{\"index\":" << particles.GetTextureHandle().index
						<< ",\"generation\":" << particles.GetTextureHandle().generation << "}}";
				}

				if (entity.HasComponent<AudioSourceComponent>()) {
					const auto& audio = entity.GetComponent<AudioSourceComponent>();
					writeSeparator();
					projectContent << "        \"AudioSource\": {\"volume\":" << audio.GetVolume()
						<< ",\"pitch\":" << audio.GetPitch()
						<< ",\"loop\":" << (audio.IsLooping() ? "true" : "false")
						<< ",\"audioHandle\":" << audio.GetAudioHandle().GetHandle() << "}";
				}

				if (entity.HasComponent<StaticTag>()) { writeSeparator(); projectContent << "        \"StaticTag\": true"; }
				if (entity.HasComponent<DisabledTag>()) { writeSeparator(); projectContent << "        \"DisabledTag\": true"; }
				if (entity.HasComponent<DeadlyTag>()) { writeSeparator(); projectContent << "        \"DeadlyTag\": true"; }
				if (entity.HasComponent<IdTag>()) { writeSeparator(); projectContent << "        \"IdTag\": true"; }

				projectContent << "\n      }\n";
				projectContent << "    }";
				if (i + 1 < handles.size()) {
					projectContent << ",";
				}
				projectContent << "\n";
			}

			projectContent << "  ]\n";
			projectContent << "}\n";

			const std::filesystem::path tempPath = outputPath.string() + ".tmp";
			{
				std::ofstream output(tempPath, std::ios::out | std::ios::trunc);
				if (!output.is_open()) {
					outError = "Unable to open save file for writing: " + outputPath.string();
					return false;
				}
				output << projectContent.str();
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
#include "pch.hpp"
#include "Scripting/ScriptComponentInspector.hpp"
#include "Scripting/ScriptComponent.hpp"
#include "Gui/ImGuiUtils.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "Core/Application.hpp"
#include "Serialization/Path.hpp"
#include "Project/ProjectManager.hpp"
#include "Scene/Scene.hpp"
#include "Scene/SceneManager.hpp"
#include "Components/General/UUIDComponent.hpp"
#include "Components/General/NameComponent.hpp"

#include <imgui.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <algorithm>
#include <vector>
#include <string>
#include <climits>

namespace Bolt {

	// ── Editor Field Info & Minimal JSON Parsing ────────────────────

	namespace {

		struct EditorFieldInfo {
			std::string name;
			std::string displayName;
			std::string type;
			std::string value;
			std::string tooltip;
			bool readOnly = false;
			bool hasClamp = false;
			float clampMin = 0.0f;
			float clampMax = 0.0f;
		};

		// ── Minimal JSON helpers ──────────────────────────────────────
		// The C# side returns a JSON array of field objects. We parse it
		// with simple string scanning — no external dependency needed.

		static void SkipWhitespace(const char*& p) {
			while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') ++p;
		}

		static std::string ParseJsonString(const char*& p) {
			SkipWhitespace(p);
			if (*p != '"') return {};
			++p; // skip opening quote
			std::string result;
			while (*p && *p != '"') {
				if (*p == '\\' && *(p + 1)) {
					++p;
					switch (*p) {
						case '"': result += '"'; break;
						case '\\': result += '\\'; break;
						case '/': result += '/'; break;
						case 'n': result += '\n'; break;
						case 't': result += '\t'; break;
						case 'r': result += '\r'; break;
						default: result += *p; break;
					}
				} else {
					result += *p;
				}
				++p;
			}
			if (*p == '"') ++p; // skip closing quote
			return result;
		}

		static std::string ParseJsonValue(const char*& p) {
			SkipWhitespace(p);
			if (*p == '"') return ParseJsonString(p);
			// Parse a bare value (number, bool, null) as string
			std::string result;
			while (*p && *p != ',' && *p != '}' && *p != ']'
				&& *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') {
				result += *p;
				++p;
			}
			return result;
		}

		static std::vector<EditorFieldInfo> ParseEditorFields(const char* json) {
			std::vector<EditorFieldInfo> fields;
			if (!json || !*json) return fields;

			const char* p = json;
			SkipWhitespace(p);
			if (*p != '[') return fields;
			++p; // skip '['

			while (*p) {
				SkipWhitespace(p);
				if (*p == ']') break;
				if (*p == ',') { ++p; continue; }
				if (*p != '{') break;
				++p; // skip '{'

				EditorFieldInfo field;

				while (*p) {
					SkipWhitespace(p);
					if (*p == '}') { ++p; break; }
					if (*p == ',') { ++p; continue; }

					std::string key = ParseJsonString(p);
					SkipWhitespace(p);
					if (*p == ':') ++p; // skip ':'

					if (key == "name")             field.name        = ParseJsonValue(p);
					else if (key == "displayName") field.displayName = ParseJsonValue(p);
					else if (key == "type")        field.type        = ParseJsonValue(p);
					else if (key == "value")       field.value       = ParseJsonValue(p);
					else if (key == "readOnly")    field.readOnly    = (ParseJsonValue(p) == "true");
					else if (key == "hasClamp")    field.hasClamp    = (ParseJsonValue(p) == "true");
					else if (key == "clampMin")    field.clampMin    = static_cast<float>(std::atof(ParseJsonValue(p).c_str()));
					else if (key == "clampMax")    field.clampMax    = static_cast<float>(std::atof(ParseJsonValue(p).c_str()));
					else if (key == "tooltip")     field.tooltip     = ParseJsonValue(p);
					else                           ParseJsonValue(p); // skip unknown keys
				}

				if (field.displayName.empty()) field.displayName = field.name;
				fields.push_back(std::move(field));
			}

			return fields;
		}

		// ── Render a single editor field ──────────────────────────────

		static void RenderEditorField(int32_t gcHandle, EditorFieldInfo& field) {
			ImGui::PushID(field.name.c_str());

			if (field.readOnly)
				ImGui::BeginDisabled();

			bool changed = false;
			std::string newValue;

			const std::string& type = field.type;
			const char* label = field.displayName.c_str();

			if (type == "float" || type == "double") {
				float val = static_cast<float>(std::atof(field.value.c_str()));
				float mn = field.hasClamp ? field.clampMin : 0.0f;
				float mx = field.hasClamp ? field.clampMax : 0.0f;
				if (ImGui::DragFloat(label, &val, 0.1f, mn, mx)) {
					changed = true;
					char buf[64];
					std::snprintf(buf, sizeof(buf), "%g", val);
					newValue = buf;
				}
			}
			else if (type == "int") {
				int val = std::atoi(field.value.c_str());
				int mn = field.hasClamp ? static_cast<int>(field.clampMin) : 0;
				int mx = field.hasClamp ? static_cast<int>(field.clampMax) : 0;
				if (ImGui::DragInt(label, &val, 1.0f, mn, mx)) {
					changed = true;
					newValue = std::to_string(val);
				}
			}
			else if (type == "short") {
				int val = std::atoi(field.value.c_str());
				int mn = field.hasClamp ? std::max(static_cast<int>(field.clampMin), -32768) : -32768;
				int mx = field.hasClamp ? std::min(static_cast<int>(field.clampMax), 32767) : 32767;
				if (ImGui::DragInt(label, &val, 1.0f, mn, mx)) {
					if (val < -32768) val = -32768;
					if (val > 32767) val = 32767;
					changed = true;
					newValue = std::to_string(val);
				}
			}
			else if (type == "byte") {
				int val = std::atoi(field.value.c_str());
				int mn = field.hasClamp ? std::max(static_cast<int>(field.clampMin), 0) : 0;
				int mx = field.hasClamp ? std::min(static_cast<int>(field.clampMax), 255) : 255;
				if (ImGui::DragInt(label, &val, 1.0f, mn, mx)) {
					if (val < 0) val = 0;
					if (val > 255) val = 255;
					changed = true;
					newValue = std::to_string(val);
				}
			}
			else if (type == "sbyte") {
				int val = std::atoi(field.value.c_str());
				int mn = field.hasClamp ? std::max(static_cast<int>(field.clampMin), -128) : -128;
				int mx = field.hasClamp ? std::min(static_cast<int>(field.clampMax), 127) : 127;
				if (ImGui::DragInt(label, &val, 1.0f, mn, mx)) {
					if (val < -128) val = -128;
					if (val > 127) val = 127;
					changed = true;
					newValue = std::to_string(val);
				}
			}
			else if (type == "uint") {
				int val = std::atoi(field.value.c_str());
				int mn = field.hasClamp ? std::max(static_cast<int>(field.clampMin), 0) : 0;
				int mx = field.hasClamp ? std::min(static_cast<int>(field.clampMax), INT_MAX) : INT_MAX;
				if (ImGui::DragInt(label, &val, 1.0f, mn, mx)) {
					if (val < 0) val = 0;
					changed = true;
					newValue = std::to_string(val);
				}
			}
			else if (type == "ushort") {
				int val = std::atoi(field.value.c_str());
				int mn = field.hasClamp ? std::max(static_cast<int>(field.clampMin), 0) : 0;
				int mx = field.hasClamp ? std::min(static_cast<int>(field.clampMax), 65535) : 65535;
				if (ImGui::DragInt(label, &val, 1.0f, mn, mx)) {
					if (val < 0) val = 0;
					if (val > 65535) val = 65535;
					changed = true;
					newValue = std::to_string(val);
				}
			}
			else if (type == "long" || type == "ulong") {
				char buf[64];
				std::strncpy(buf, field.value.c_str(), sizeof(buf) - 1);
				buf[sizeof(buf) - 1] = '\0';
				if (ImGui::InputText(label, buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue)) {
					changed = true;
					newValue = buf;
				}
			}
			else if (type == "bool") {
				bool val = (field.value == "true" || field.value == "True" || field.value == "1");
				if (ImGui::Checkbox(label, &val)) {
					changed = true;
					newValue = val ? "true" : "false";
				}
			}
			else if (type == "string") {
				char buf[256];
				std::strncpy(buf, field.value.c_str(), sizeof(buf) - 1);
				buf[sizeof(buf) - 1] = '\0';
				if (ImGui::InputText(label, buf, sizeof(buf))) {
					changed = true;
					newValue = buf;
				}
			}
			else if (type == "color") {
				// Parse "r,g,b,a" format
				float col[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
				if (!field.value.empty()) {
					std::sscanf(field.value.c_str(), "%f,%f,%f,%f",
						&col[0], &col[1], &col[2], &col[3]);
				}
				if (ImGui::ColorEdit4(label, col)) {
					changed = true;
					char buf[128];
					std::snprintf(buf, sizeof(buf), "%g,%g,%g,%g",
						col[0], col[1], col[2], col[3]);
					newValue = buf;
				}
			}
			else if (type == "entity") {
				uint64_t uuid = std::strtoull(field.value.c_str(), nullptr, 10);

				std::string displayName = "(None)";
				if (uuid != 0) {
					Scene* scene = ScriptEngine::GetScene();
					if (scene) {
						auto view = scene->GetRegistry().view<UUIDComponent, NameComponent>();
						for (auto [ent, uuidComp, nameComp] : view.each()) {
							if (static_cast<uint64_t>(uuidComp.Id) == uuid) {
								displayName = nameComp.Name;
								break;
							}
						}
						if (displayName == "(None)") displayName = "(Missing Entity)";
					}
				}

				ImGui::Text("%s: %s", label, displayName.c_str());

				if (uuid != 0) {
					ImGui::SameLine();
					if (ImGui::SmallButton("X##ClearEntity")) {
						newValue = "0";
						changed = true;
					}
				}

				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY")) {
						struct HierarchyDragData { int Index; uint32_t EntityHandle; };
						auto* data = static_cast<const HierarchyDragData*>(payload->Data);
						EntityHandle handle = static_cast<EntityHandle>(data->EntityHandle);
						Scene* scene = ScriptEngine::GetScene();
						if (scene && scene->IsValid(handle) && scene->HasComponent<UUIDComponent>(handle)) {
							uint64_t droppedUUID = static_cast<uint64_t>(scene->GetComponent<UUIDComponent>(handle).Id);
							newValue = std::to_string(droppedUUID);
							changed = true;
						}
					}
					ImGui::EndDragDropTarget();
				}
			}
			else if (type == "texture") {
				std::string path = field.value;
				std::string displayName = path.empty() ? "(None)" : std::filesystem::path(path).filename().string();

				bool exists = path.empty() || std::filesystem::exists(path);
				if (!path.empty() && !exists) {
					ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%s: (Missing: %s)", label, displayName.c_str());
				} else {
					ImGui::Text("%s: %s", label, displayName.c_str());
				}

				if (!path.empty()) {
					ImGui::SameLine();
					if (ImGui::SmallButton("X##ClearTex")) { newValue = ""; changed = true; }
				}

				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
						std::string droppedPath(static_cast<const char*>(payload->Data));
						std::string ext = std::filesystem::path(droppedPath).extension().string();
						std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
						if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga") {
							newValue = droppedPath;
							changed = true;
						}
					}
					ImGui::EndDragDropTarget();
				}
			}
			else if (type == "audio") {
				std::string path = field.value;
				std::string displayName = path.empty() ? "(None)" : std::filesystem::path(path).filename().string();

				bool exists = path.empty() || std::filesystem::exists(path);
				if (!path.empty() && !exists) {
					ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%s: (Missing: %s)", label, displayName.c_str());
				} else {
					ImGui::Text("%s: %s", label, displayName.c_str());
				}

				if (!path.empty()) {
					ImGui::SameLine();
					if (ImGui::SmallButton("X##ClearAudio")) { newValue = ""; changed = true; }
				}

				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
						std::string droppedPath(static_cast<const char*>(payload->Data));
						std::string ext = std::filesystem::path(droppedPath).extension().string();
						std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
						if (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac") {
							newValue = droppedPath;
							changed = true;
						}
					}
					ImGui::EndDragDropTarget();
				}
			}
			else if (type.rfind("component:", 0) == 0) {
				std::string compTypeName = type.substr(10);
				std::string val = field.value;
				std::string displayName = "(None)";

				if (!val.empty()) {
					auto sep = val.find(':');
					if (sep != std::string::npos) {
						uint64_t entityId = std::strtoull(val.substr(0, sep).c_str(), nullptr, 10);
						std::string compName = val.substr(sep + 1);
						Scene* scene = ScriptEngine::GetScene();
						if (scene && entityId != 0) {
							auto view = scene->GetRegistry().view<UUIDComponent, NameComponent>();
							bool found = false;
							for (auto [ent, uuidComp, nameComp] : view.each()) {
								if (static_cast<uint64_t>(uuidComp.Id) == entityId) {
									displayName = nameComp.Name + "." + compName;
									found = true;
									break;
								}
							}
							if (!found) displayName = "(Missing)." + compName;
						}
					}
				}

				ImGui::Text("%s: %s", label, displayName.c_str());

				if (!val.empty()) {
					ImGui::SameLine();
					if (ImGui::SmallButton("X##ClearComp")) { newValue = ""; changed = true; }
				}

				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("COMPONENT_REF")) {
						std::string refStr(static_cast<const char*>(payload->Data));
						newValue = refStr;
						changed = true;
					}
					ImGui::EndDragDropTarget();
				}
			}
			else {
				// Unknown type: show read-only text
				ImGui::TextDisabled("%s: %s (%s)", label, field.value.c_str(), type.c_str());
			}

			if (field.readOnly)
				ImGui::EndDisabled();

			if (!field.tooltip.empty() && ImGui::IsItemHovered()) {
				ImGui::SetTooltip("%s", field.tooltip.c_str());
			}

			if (changed) {
				field.value = newValue;
			}

			ImGui::PopID();
		}

		// ── Render all fields for a script ────────────────────────────
		// Works in both Edit Mode (no instance) and Play Mode (live instance).

		static void RenderScriptFieldsForInstance(ScriptComponent& sc, const ScriptInstance& instance) {
			auto& callbacks = ScriptEngine::GetCallbacks();
			bool isPlaying = Application::GetIsPlaying();
			bool hasLiveInstance = instance.HasManagedInstance();

			const char* json = nullptr;

			if (hasLiveInstance && callbacks.GetScriptFields) {
				// Play mode: get live field values from the running instance
				json = callbacks.GetScriptFields(static_cast<int32_t>(instance.GetGCHandle()));
			} else if (callbacks.GetClassFieldDefs) {
				// Edit mode: get field definitions (with defaults) from the class
				json = callbacks.GetClassFieldDefs(instance.GetClassName().c_str());
			}

			if (!json || !*json || (json[0] == '[' && json[1] == ']')) return;

			auto fields = ParseEditorFields(json);
			if (fields.empty()) return;

			// In edit mode, overlay saved values from PendingFieldValues
			if (!hasLiveInstance) {
				for (auto& field : fields) {
					std::string key = instance.GetClassName() + "." + field.name;
					auto it = sc.PendingFieldValues.find(key);
					if (it != sc.PendingFieldValues.end())
						field.value = it->second;
				}
			}

			ImGui::Indent(8.0f);
			for (auto& field : fields) {
				std::string oldValue = field.value;

				if (hasLiveInstance) {
					RenderEditorField(static_cast<int32_t>(instance.GetGCHandle()), field);
				} else {
					// Edit mode: render with a dummy handle (changes go to PendingFieldValues)
					RenderEditorField(0, field);
				}

				// Detect value change
				if (field.value != oldValue) {
					if (hasLiveInstance && callbacks.SetScriptField) {
						callbacks.SetScriptField(
							static_cast<int32_t>(instance.GetGCHandle()),
							field.name.c_str(), field.value.c_str());
					}

					if (!isPlaying) {
						// Edit mode: persist to PendingFieldValues and mark dirty
						std::string key = instance.GetClassName() + "." + field.name;
						sc.PendingFieldValues[key] = field.value;
					}
					// Play mode: transient change — do NOT mark dirty
				}
			}
			ImGui::Unindent(8.0f);
		}

	} // end anonymous namespace forward declarations

	// ── Script Picker (same pattern as Texture Picker in ComponentInspectors.cpp) ──

	namespace {
		struct ScriptPickerEntry {
			std::string ClassName;   // stem without extension
			std::string RelativePath;
			std::string Extension;   // ".cs" or ".cpp"
		};

		static bool s_ScriptPickerOpen = false;
		static char s_ScriptPickerSearch[128] = {};
		static std::vector<ScriptPickerEntry> s_ScriptPickerEntries;
		// Store entity handle instead of raw component pointer to avoid
		// dangling pointer if the entity is destroyed or components reallocate.
		static EntityHandle s_ScriptPickerTargetEntity = entt::null;

		static bool IsScriptExtension(const std::string& ext) {
			return ext == ".cs" || ext == ".cpp";
		}

		static void CollectScriptFiles(const std::filesystem::path& dir,
			std::vector<ScriptPickerEntry>& entries) {
			if (!std::filesystem::exists(dir)) return;
			for (const auto& entry : std::filesystem::recursive_directory_iterator(dir)) {
				if (!entry.is_regular_file()) continue;
				std::string ext = entry.path().extension().string();
				std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
				if (!IsScriptExtension(ext)) continue;

				ScriptPickerEntry e;
				e.ClassName = entry.path().stem().string();
				e.RelativePath = entry.path().string();
				e.Extension = ext;
				entries.push_back(std::move(e));
			}
			std::sort(entries.begin(), entries.end(),
				[](const ScriptPickerEntry& a, const ScriptPickerEntry& b) {
					return a.ClassName < b.ClassName;
				});
		}

		static void OpenScriptPicker(EntityHandle entity) {
			s_ScriptPickerOpen = true;
			s_ScriptPickerSearch[0] = '\0';
			s_ScriptPickerTargetEntity = entity;
			s_ScriptPickerEntries.clear();

			BoltProject* project = ProjectManager::GetCurrentProject();
			if (project) {
				// C# scripts in Assets/Scripts/
				CollectScriptFiles(std::filesystem::path(project->ScriptsDirectory), s_ScriptPickerEntries);
				// C++ native scripts
				CollectScriptFiles(std::filesystem::path(project->NativeSourceDir), s_ScriptPickerEntries);
			}
			else {
				// Dev/sandbox fallback
				std::string base = Path::ExecutableDir();
				CollectScriptFiles(std::filesystem::path(base) / ".." / ".." / ".." / "Bolt-Sandbox" / "Source", s_ScriptPickerEntries);
				CollectScriptFiles(std::filesystem::path(base) / ".." / ".." / ".." / "Bolt-NativeScripts" / "Source", s_ScriptPickerEntries);
			}
		}

		static void RenderScriptPicker() {
			if (!s_ScriptPickerOpen) return;

			ImGui::SetNextWindowSize(ImVec2(320, 380), ImGuiCond_FirstUseEver);
			if (!ImGui::Begin("Select Script", &s_ScriptPickerOpen)) {
				ImGui::End();
				return;
			}

			ImGui::SetNextItemWidth(-1);
			ImGui::InputTextWithHint("##ScriptSearch", "Search...", s_ScriptPickerSearch, sizeof(s_ScriptPickerSearch));
			ImGui::Separator();

			ImGui::BeginChild("##ScriptList");

			std::string filter(s_ScriptPickerSearch);
			std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

			for (const auto& entry : s_ScriptPickerEntries) {
				if (!filter.empty()) {
					std::string lowerName = entry.ClassName;
					std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
					if (lowerName.find(filter) == std::string::npos) continue;
				}

				// Show class name with extension tag
				std::string label = entry.ClassName + "  " + entry.Extension;
				if (ImGui::Selectable(label.c_str(), false)) {
					Scene* scene = ScriptEngine::GetScene();
					if (!scene) scene = SceneManager::Get().GetActiveScene();
					if (scene && scene->IsValid(s_ScriptPickerTargetEntity)
						&& scene->HasComponent<ScriptComponent>(s_ScriptPickerTargetEntity)) {
						auto& sc = scene->GetComponent<ScriptComponent>(s_ScriptPickerTargetEntity);
						if (!sc.HasScript(entry.ClassName)) {
							sc.AddScript(entry.ClassName);
						}
					}
					s_ScriptPickerOpen = false;
				}

				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("%s", entry.RelativePath.c_str());
				}
			}

			ImGui::EndChild();
			ImGui::End();
		}
	}

	// ── Inspector ───────────────────────────────────────────────────

	void DrawScriptComponentInspector(Entity entity)
	{
		auto& scriptComp = entity.GetComponent<ScriptComponent>();

		size_t removeIndex = SIZE_MAX;

		// Each script rendered as its own component section
		for (size_t i = 0; i < scriptComp.Scripts.size(); i++) {
			auto& instance = scriptComp.Scripts[i];
			std::string label = instance.GetClassName().empty() ? "Script" : instance.GetClassName();

			ImGui::PushID(static_cast<int>(i));

			bool removeRequested = false;
			bool open = ImGuiUtils::BeginComponentSection(label.c_str(), removeRequested);

			if (removeRequested) {
				removeIndex = i;
			}

			if (open) {
				if (instance.IsBound()) {
					ImGui::TextDisabled("Bound | %s", instance.HasStarted() ? "Started" : "Pending Start");
				}

				// Show [ShowInEditor] fields in both Edit and Play mode
				if (ScriptEngine::IsInitialized()) {
					RenderScriptFieldsForInstance(scriptComp, instance);
				}

				ImGuiUtils::EndComponentSection();
			}

			ImGui::PopID();
		}

		if (removeIndex != SIZE_MAX) {
			scriptComp.RemoveScript(removeIndex);
		}

		RenderScriptPicker();
	}

}

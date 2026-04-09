#include "pch.hpp"
#include "Scripting/ScriptComponentInspector.hpp"
#include "Scripting/ScriptComponent.hpp"
#include "Serialization/Path.hpp"
#include "Project/ProjectManager.hpp"

#include <imgui.h>
#include <cstdio>
#include <filesystem>
#include <algorithm>

namespace Bolt {

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
		static ScriptComponent* s_ScriptPickerTarget = nullptr;

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

		static void OpenScriptPicker(ScriptComponent& target) {
			s_ScriptPickerOpen = true;
			s_ScriptPickerSearch[0] = '\0';
			s_ScriptPickerTarget = &target;
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
					if (s_ScriptPickerTarget && !s_ScriptPickerTarget->HasScript(entry.ClassName)) {
						s_ScriptPickerTarget->AddScript(entry.ClassName);
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

		for (size_t i = 0; i < scriptComp.Scripts.size(); i++) {
			auto& instance = scriptComp.Scripts[i];

			ImGui::PushID(static_cast<int>(i));

			char buffer[256]{};
			std::snprintf(buffer, sizeof(buffer), "%s", instance.GetClassName().c_str());

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 30.0f);
			if (ImGui::InputText("##ClassName", buffer, sizeof(buffer))) {
				instance.SetClassName(buffer);
			}

			ImGui::SameLine();
			if (ImGui::SmallButton("X")) {
				removeIndex = i;
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Remove this script");
			}

			if (instance.IsBound()) {
				ImGui::TextDisabled("  Bound | %s", instance.HasStarted() ? "Started" : "Pending Start");
			}

			ImGui::PopID();
		}

		if (removeIndex != SIZE_MAX) {
			scriptComp.RemoveScript(removeIndex);
		}

		ImGui::Spacing();

		if (ImGui::Button("Add Script", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
			OpenScriptPicker(scriptComp);
		}

		RenderScriptPicker();

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
				std::string droppedPath(static_cast<const char*>(payload->Data));
				std::string ext = std::filesystem::path(droppedPath).extension().string();
				std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

				if (ext == ".cs" || ext == ".cpp" || ext == ".h" || ext == ".hpp") {
					std::string className = std::filesystem::path(droppedPath).stem().string();
					if (!scriptComp.HasScript(className)) {
						scriptComp.AddScript(className);
					}
				}
			}
			ImGui::EndDragDropTarget();
		}
	}

}

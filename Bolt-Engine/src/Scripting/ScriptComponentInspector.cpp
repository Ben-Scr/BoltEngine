#include "pch.hpp"
#include "Scripting/ScriptComponentInspector.hpp"
#include "Scripting/ScriptComponent.hpp"

#include <imgui.h>
#include <cstdio>
#include <filesystem>
#include <algorithm>

namespace Bolt {

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
			scriptComp.AddScript("NewScript");
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
				std::string droppedPath(static_cast<const char*>(payload->Data));
				std::string ext = std::filesystem::path(droppedPath).extension().string();
				std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

				if (ext == ".cs") {
					// (Ben-Scr) Derive class name from filename (e.g. "PlayerController.cs" -> "PlayerController")
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

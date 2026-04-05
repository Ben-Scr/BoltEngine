#pragma once
#include <imgui.h>
#include <magic_enum/magic_enum.hpp>
#include <type_traits>

namespace Bolt {
namespace ImGuiUtils {

	template<typename TEnum, typename Setter>
	bool DrawEnumCombo(const char* label, TEnum currentValue, Setter&& setter)
	{
		static_assert(std::is_enum_v<TEnum>, "DrawEnumCombo requires an enum type.");

		auto preview = magic_enum::enum_name(currentValue);
		if (preview.empty()) {
			preview = "Unknown";
		}

		bool changed = false;

		if (ImGui::BeginCombo(label, preview.data())) {
			for (TEnum value : magic_enum::enum_values<TEnum>()) {
				const bool isSelected = (currentValue == value);

				auto name = magic_enum::enum_name(value);
				if (name.empty()) {
					continue;
				}

				if (ImGui::Selectable(name.data(), isSelected)) {
					setter(value);
					changed = true;
				}

				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}

		return changed;
	}

	template<typename TVec2>
	void DrawVec2ReadOnly(const char* id, const TVec2& vec2)
	{
		ImGui::PushID(id);

		float values[2] = { static_cast<float>(vec2.x), static_cast<float>(vec2.y) };

		ImGui::BeginGroup();
		ImGui::PushItemWidth(90.0f);

		ImGui::BeginDisabled();
		ImGui::InputFloat("##X", &values[0], 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_ReadOnly);
		ImGui::SameLine();
		ImGui::InputFloat("##Y", &values[1], 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_ReadOnly);
		ImGui::EndDisabled();

		ImGui::PopItemWidth();

		ImGui::SameLine();
		ImGui::TextUnformatted(id);

		ImGui::EndGroup();
		ImGui::PopID();
	}

	void DrawTexturePreview(unsigned int rendererId, float texWidth, float texHeight, float previewSize = 96.0f);

	// Draws a collapsing component header with right-click "Remove Component" support.
	// Returns true if the section is open (caller should draw component fields).
	// Sets `removeRequested` to true if the user clicked "Remove Component".
	bool BeginComponentSection(const char* label, bool& removeRequested);

} // namespace ImGuiUtils
} // namespace Bolt

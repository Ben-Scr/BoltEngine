#pragma once
#include <imgui.h>
#include "Collections/Color.hpp"
#include "Collections/Vec2.hpp"
#include <magic_enum/magic_enum.hpp>
#include <string>
#include <type_traits>

namespace Bolt::ImGuiUtils {

	inline Color DrawColorPick4(const char* label, const Color& color)
	{
		float values[4] = { color.r, color.g, color.b, color.a };
		ImGui::ColorEdit4(label, values);
		return Color(values[0], values[1], values[2], values[3]);
	}
	template<typename Draw>
	void DrawEnabled(bool enabled, Draw&& draw) {
				if (!enabled) {
			ImGui::BeginDisabled();
		}

		draw();

		if (!enabled) {
			ImGui::EndDisabled();
		}
	}

	template<typename TEnum, typename Setter>
	bool DrawEnumCombo(const char* label, TEnum currentValue, Setter&& setter)
	{
		static_assert(std::is_enum_v<TEnum>, "DrawEnumCombo requires an enum type.");

		auto previewView = magic_enum::enum_name(currentValue);
		const char* preview = previewView.empty() ? "Unknown" : previewView.data();

		bool changed = false;

		if (ImGui::BeginCombo(label, preview)) {
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

	template<typename TVec2>
	bool DrawVec2(const char* id, TVec2& vec2)
	{
		ImGui::PushID(id);

		float values[2] = { vec2.x, vec2.y };
		bool changed = false;

		ImGui::BeginGroup();
		ImGui::PushItemWidth(90.0f);

		changed |= ImGui::InputFloat("##X", &values[0], 0.0f, 0.0f, "%.3f");
		ImGui::SameLine();
		changed |= ImGui::InputFloat("##Y", &values[1], 0.0f, 0.0f, "%.3f");

		ImGui::PopItemWidth();

		ImGui::SameLine();
		ImGui::TextUnformatted(id);

		ImGui::EndGroup();
		ImGui::PopID();

		if (changed) {
			vec2.x = values[0];
			vec2.y = values[1];
		}

		return changed;
	}

	void DrawTexturePreview(unsigned int rendererId, float texWidth, float texHeight, float previewSize = 96.0f);
	std::string Ellipsize(const std::string& text, float maxWidth, bool* outTruncated = nullptr);
	void TextEllipsis(const std::string& text, float maxWidth = -1.0f);
	void TextDisabledEllipsis(const std::string& text, float maxWidth = -1.0f);
	bool SelectableEllipsis(const std::string& text, const char* id, bool selected = false,
		ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0.0f, 0.0f), float maxWidth = -1.0f);
	bool MenuItemEllipsis(const std::string& text, const char* id,
		const char* shortcut = nullptr, bool selected = false, bool enabled = true, float maxWidth = -1.0f);

	bool BeginComponentSection(const char* label, bool& removeRequested);

	void EndComponentSection();

}

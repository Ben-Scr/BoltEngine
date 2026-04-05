#include <pch.hpp>
#include "Gui/ImGuiUtils.hpp"
#include <imgui.h>

namespace Bolt {
namespace ImGuiUtils {

	void DrawTexturePreview(unsigned int rendererId, float texWidth, float texHeight, float previewSize)
	{
		const ImVec2 previewMin = ImGui::GetCursorScreenPos();
		const ImVec2 previewMax = ImVec2(previewMin.x + previewSize, previewMin.y + previewSize);

		ImDrawList* drawList = ImGui::GetWindowDrawList();

		drawList->AddRectFilled(previewMin, previewMax, IM_COL32(35, 35, 35, 255), 6.0f);

		const float checkerSize = 8.0f;
		for (float y = previewMin.y; y < previewMax.y; y += checkerSize) {
			for (float x = previewMin.x; x < previewMax.x; x += checkerSize) {
				const int ix = static_cast<int>((x - previewMin.x) / checkerSize);
				const int iy = static_cast<int>((y - previewMin.y) / checkerSize);
				const bool even = ((ix + iy) % 2) == 0;

				drawList->AddRectFilled(
					ImVec2(x, y),
					ImVec2(
						(x + checkerSize < previewMax.x) ? x + checkerSize : previewMax.x,
						(y + checkerSize < previewMax.y) ? y + checkerSize : previewMax.y
					),
					even ? IM_COL32(70, 70, 70, 255) : IM_COL32(100, 100, 100, 255)
				);
			}
		}

		float drawWidth = previewSize;
		float drawHeight = previewSize;

		if (texWidth > 0.0f && texHeight > 0.0f) {
			const float aspect = texWidth / texHeight;
			if (aspect > 1.0f) {
				drawHeight = previewSize / aspect;
			}
			else {
				drawWidth = previewSize * aspect;
			}
		}

		const ImVec2 imageMin = ImVec2(
			previewMin.x + (previewSize - drawWidth) * 0.5f,
			previewMin.y + (previewSize - drawHeight) * 0.5f
		);
		const ImVec2 imageMax = ImVec2(imageMin.x + drawWidth, imageMin.y + drawHeight);

		drawList->AddImage(
			(ImTextureID)(intptr_t)rendererId,
			imageMin, imageMax,
			ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f)
		);

		ImGui::Dummy(ImVec2(previewSize, previewSize));
	}

	bool BeginComponentSection(const char* label, bool& removeRequested)
	{
		removeRequested = false;

		ImGui::PushID(label);
		ImGui::SeparatorText(label);

		if (ImGui::BeginPopupContextItem("##ComponentContext")) {
			if (ImGui::MenuItem("Remove Component")) {
				removeRequested = true;
			}
			ImGui::EndPopup();
		}

		ImGui::PopID();

		return true;
	}

} // namespace ImGuiUtils
} // namespace Bolt

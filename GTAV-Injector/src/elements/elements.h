#pragma once
#include "imgui_internal.h"

namespace ImGui::Custom
{
	bool SelectableRightClick(const char* label, bool selected = false, ImGuiSelectableFlags flags = 0, const ImVec2& size_arg = ImVec2(0, 0));
}

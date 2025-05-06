#pragma once
#include <imgui.h>
#include <concepts>

namespace riff::util {
template <std::same_as<float>... Ts>
void align_right(float const width, Ts const... widths) {
	auto const spacing = float(sizeof...(widths)) * ImGui::GetStyle().ItemSpacing.x;
	auto const total_width = width + (spacing + ... + widths);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - total_width);
}
} // namespace riff::util

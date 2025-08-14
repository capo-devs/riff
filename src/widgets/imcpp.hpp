#pragma once
#include <imgui.h>
#include <klib/c_string.hpp>
#include <concepts>
#include <span>
#include <string_view>
#include <vector>

namespace riff::imcpp {
class InputText {
  public:
	static constexpr std::size_t init_size_v{64};

	auto update(klib::CString name, ImVec2 multi_size = {}) -> bool;
	void set_text(std::string_view text);

	[[nodiscard]] auto as_view() const -> std::string_view { return m_buffer.data(); }
	[[nodiscard]] auto as_span() const -> std::span<char const> { return m_buffer; }

  protected:
	auto on_callback(ImGuiInputTextCallbackData& data) -> int;

	void resize_buffer(ImGuiInputTextCallbackData& data);

	std::vector<char> m_buffer{};
};

[[nodiscard]] auto begin_modal(klib::CString label) -> bool;

template <std::same_as<float>... Ts>
void align_right(float const width, Ts const... widths) {
	auto const spacing = float(sizeof...(widths)) * ImGui::GetStyle().ItemSpacing.x;
	auto const total_width = width + (spacing + ... + widths);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - total_width);
}
} // namespace riff::imcpp

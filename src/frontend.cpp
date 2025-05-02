#include <IconsKenney.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <capo/format.hpp>
#include <frontend.hpp>
#include <klib/fixed_string.hpp>
#include <cassert>
#include <ranges>

namespace riff {
namespace {
auto const duration_0_str = capo::format_duration(0s);

template <std::same_as<float>... Ts>
void align_right(float const width, Ts const... widths) {
	auto const total_width = (width + ... + widths) + (float(sizeof...(Ts)) * ImGui::GetStyle().ItemSpacing.x);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - total_width);
}
} // namespace

Frontend::Frontend(State* state) : m_state(state), m_duration_str(duration_0_str) {
	assert(m_state);
	m_state->track_changed.connect([this] {
		if (!m_state->now_playing) {
			set_now_playing({});
			return;
		}
		set_now_playing(m_state->playlist.at(*m_state->now_playing));
	});
}

void Frontend::set_now_playing(Track const& track) {
	m_duration_str.clear();
	capo::format_duration_to(m_duration_str, track.duration);
	m_seeking = false;
	m_fcursor = 0.0f;
}

void Frontend::draw() {
	update_dynamic();
	draw_menu_bar();
	auto const* active_track = m_state->active_track();
	auto const* title = active_track != nullptr ? active_track->name.c_str() : "[none]";
	ImGui::TextUnformatted(title);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
	draw_controls(active_track);
	draw_playlist();
}

void Frontend::update_dynamic() {
	if (!m_seeking && m_state->is_playing) { m_fcursor = m_state->cursor.count(); }
	m_cursor_str.clear();
	capo::format_duration_to(m_cursor_str, Time{m_fcursor});
}

void Frontend::draw_controls(Track const* active_track) {
	draw_buttons(active_track);
	draw_seekbar(active_track != nullptr ? active_track->duration : 0s);
}

void Frontend::draw_buttons(Track const* active_track) {
	ImGui::SetNextItemWidth(50.0f);
	if (active_track == nullptr) { ImGui::BeginDisabled(); }
	auto const* play_str = m_state->is_playing ? ICON_KI_PAUSE : ICON_KI_CARET_RIGHT;
	if (ImGui::ButtonEx(play_str, {50.0f, 50.0f})) {
		if (m_state->is_playing) {
			m_state->on_pause.dispatch();
		} else {
			m_state->on_play.dispatch();
		}
	}
	if (active_track == nullptr) { ImGui::EndDisabled(); }

	static constexpr auto volume_width_v = 100.0f;
	static constexpr auto balance_width_v = 100.0f;
	align_right(balance_width_v);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 50.0f);
	ImGui::SetNextItemWidth(balance_width_v);
	if (ImGui::SliderFloat("##balance", &m_state->balance, -1.0f, 1.0f, "%.1f")) {
		m_state->balance_changed.dispatch();
	}
	align_right(volume_width_v);
	ImGui::SetNextItemWidth(volume_width_v);
	m_volume = int(m_state->gain * 100.0f);
	if (ImGui::SliderInt("##volume", &m_volume, 0, 100, "%d", ImGuiSliderFlags_ClampZeroRange)) {
		m_state->gain = float(m_volume) * 0.01f;
		m_state->gain_changed.dispatch();
	}
}

void Frontend::draw_seekbar(Time const track_duration) {
	ImGui::NewLine();
	ImGui::TextUnformatted(duration_0_str.c_str());
	auto const duration_width = ImGui::CalcTextSize(m_duration_str.c_str()).x;
	ImGui::SameLine();
	align_right(duration_width);
	ImGui::TextUnformatted(m_duration_str.c_str());

	auto fduration = track_duration.count();
	if (track_duration == 0s) { ImGui::BeginDisabled(); }
	ImGui::SetNextItemWidth(-1.0f);
	static constexpr auto flags_v = ImGuiSliderFlags_NoInput;
	ImGui::SliderFloat("##cursor", &m_fcursor, 0.0f, track_duration.count(), m_cursor_str.c_str(), flags_v);
	if (ImGui::IsItemClicked()) { m_seeking = true; }
	auto const was_seeking = m_seeking;
	if (m_seeking && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) { m_seeking = false; }
	if (was_seeking && !m_seeking) {
		m_state->cursor = Time{m_fcursor};
		m_state->on_seek.dispatch();
	}
	if (track_duration == 0s) { ImGui::EndDisabled(); }
}

void Frontend::draw_menu_bar() {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			ImGui::Separator();
			if (ImGui::MenuItem("Quit")) { m_state->on_quit.dispatch(); }
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void Frontend::draw_playlist() {
	ImGui::Separator();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
	ImGui::TextUnformatted(ICON_KI_LIST);
	ImGui::BeginChild("playlist", {}, ImGuiChildFlags_Borders);
	for (auto const [index, track] : std::views::enumerate(m_state->playlist)) {
		auto const is_now_playing = m_state->now_playing && *m_state->now_playing == index;
		if (is_now_playing) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0f, 1.0f, 0.0f, 1.0f}); }
		auto const is_selected = m_selected == index;
		if (ImGui::Selectable(track.name.c_str(), m_selected == index)) {
			m_selected = is_selected ? -1 : std::int32_t(index);
		}
		if (is_now_playing) { ImGui::PopStyleColor(); }
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			m_state->now_playing = index;
			m_state->on_track_select.dispatch();
		}
	}
	ImGui::EndChild();
}
} // namespace riff

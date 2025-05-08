#include <IconsKenney.h>
#include <imgui.h>
#include <tracklist.hpp>
#include <util.hpp>
#include <algorithm>
#include <array>
#include <filesystem>
#include <utility>

namespace riff {
namespace {
namespace fs = std::filesystem;

[[nodiscard]] auto to_track(fs::path const& path, std::uint64_t& out_prev_id) {
	auto ret = Track{.path = path.generic_string(), .name = path.filename().generic_string()};
	ret.label = std::format("{}##{}", ret.name, ++out_prev_id);
	return ret;
}

[[nodiscard]] auto is_music(fs::path const& path) {
	static constexpr auto extensions_v = std::array{".wav", ".mp3", ".flac"};
	auto const extension = path.extension().generic_string();
	return std::ranges::find(extensions_v, extension) != extensions_v.end();
}
} // namespace

auto Tracklist::has_playable_track() const -> bool {
	return std::ranges::any_of(m_tracks, [](Track const& t) { return t.status != Track::Status::Error; });
}

auto Tracklist::has_next_track() const -> bool {
	if (m_tracks.empty()) { return false; }
	if (is_inactive()) { return true; }
	if (!is_last()) { return true; }
	return false;
}

auto Tracklist::push(klib::CString const path) -> bool {
	auto const fs_path = fs::path{path.as_view()};
	if (!is_music(fs_path)) { return false; }
	m_tracks.push_back(to_track(fs_path, m_prev_id));
	return true;
}

void Tracklist::clear() {
	m_tracks.clear();
	m_active = m_cursor = m_tracks.end();
}

auto Tracklist::cycle_next() -> Track* {
	if (m_tracks.empty()) { return nullptr; }
	if (is_inactive() || is_last()) {
		m_active = m_tracks.begin();
	} else {
		++m_active;
	}
	return &*m_active;
}

auto Tracklist::cycle_prev() -> Track* {
	if (m_tracks.empty()) { return nullptr; }
	if (is_first()) { m_active = m_tracks.end(); }
	--m_active;
	return &*m_active;
}

void Tracklist::update(IMediator& mediator) {
	ImGui::TextUnformatted(ICON_KI_LIST);
	auto const none_selected = m_cursor == m_tracks.end();
	if (none_selected) { ImGui::BeginDisabled(); }
	ImGui::SameLine();
	remove_track(mediator);
	ImGui::SameLine();
	move_track_up();
	ImGui::SameLine();
	move_track_down();
	if (none_selected) { ImGui::EndDisabled(); }
	track_list(mediator);
}

auto Tracklist::is_inactive() const -> bool { return m_active == m_tracks.end(); }

auto Tracklist::is_first() const -> bool { return m_active == m_tracks.begin(); }

auto Tracklist::is_last() const -> bool {
	assert(is_inactive());
	return std::next(m_active) == m_tracks.end();
}

void Tracklist::remove_track(IMediator& mediator) {
	if (ImGui::Button(ICON_KI_TIMES)) {
		if (m_active == m_cursor) {
			mediator.unload_active();
			m_active = m_tracks.end();
		}
		m_cursor = m_tracks.erase(m_cursor);
	}
}

void Tracklist::move_track_up() {
	auto const is_first = m_cursor == m_tracks.begin();
	if (is_first) { ImGui::BeginDisabled(); }
	if (ImGui::Button(ICON_KI_ARROW_TOP)) { swap_with_cursor(std::prev(m_cursor)); }
	if (is_first) { ImGui::EndDisabled(); }
}

void Tracklist::move_track_down() {
	auto const is_last = !m_tracks.empty() && std::next(m_cursor) == m_tracks.end();
	if (is_last) { ImGui::BeginDisabled(); }
	if (ImGui::Button(ICON_KI_ARROW_BOTTOM)) { swap_with_cursor(std::next(m_cursor)); }
	if (is_last) { ImGui::EndDisabled(); }
}

void Tracklist::track_list(IMediator& mediator) {
	auto switch_track = false;
	ImGui::BeginChild("Tracklist", {}, ImGuiChildFlags_Borders);
	for (auto it = m_tracks.begin(); it != m_tracks.end(); ++it) {
		auto const& track = *it;
		auto const is_now_playing = m_active == it;
		auto const is_error = track.status == Track::Status::Error;
		if (is_error) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0f, 0.3f, 0.0f, 1.0f});
		} else if (is_now_playing) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.5f, 1.0f, 0.2f, 1.0f});
		}
		auto const is_selected = m_cursor == it;
		if (ImGui::Selectable(track.label.c_str(), is_selected)) { m_cursor = it; }
		if (is_now_playing || is_error) { ImGui::PopStyleColor(); }
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) { switch_track = true; }
		if (track.status == Track::Status::Ok) {
			ImGui::SameLine();
			auto const width = ImGui::CalcTextSize(track.duration_label.c_str()).x;
			util::align_right(width);
			ImGui::TextUnformatted(track.duration_label.c_str());
		}
	}
	ImGui::EndChild();

	if (switch_track) {
		auto& track = *m_cursor;
		m_active = mediator.play_track(track) ? m_cursor : m_tracks.end();
	}
}

void Tracklist::swap_with_cursor(It const it) {
	assert(m_tracks.size() > 1 && m_cursor != m_tracks.end() && it != m_tracks.end());
	if (m_active == it) {
		m_active = m_cursor;
	} else if (m_active == m_cursor) {
		m_active = it;
	}
	std::swap(*m_cursor, *it);
	m_cursor = it;
}
} // namespace riff

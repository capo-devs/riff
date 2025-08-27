#include <IconsKenney.h>
#include <imgui.h>
#include <types/playlist.hpp>
#include <widgets/imcpp.hpp>
#include <widgets/tracklist.hpp>
#include <algorithm>
#include <array>
#include <cassert>
#include <filesystem>
#include <utility>

namespace riff {
namespace {
namespace fs = std::filesystem;

enum class FileType : std::int8_t { Unknown, Music, Playlist };

[[nodiscard]] constexpr auto get_file_type(std::string_view const extension) {
	static constexpr auto music_v = std::array{".wav", ".mp3", ".flac"};
	if (std::ranges::find(music_v, extension) != music_v.end()) { return FileType::Music; }
	static constexpr auto playlist_v = std::array{".m3u", ".m3u8"};
	if (std::ranges::find(playlist_v, extension) != playlist_v.end()) { return FileType::Playlist; }
	return FileType::Unknown;
}

[[nodiscard]] auto to_track(fs::path const& path, std::uint64_t& out_prev_id) {
	auto ret = Track{.path = path.generic_string(), .name = path.filename().generic_string()};
	ret.label = std::format("{}##{}", ret.name, ++out_prev_id);
	return ret;
}
} // namespace

auto Tracklist::has_playable_track() const -> bool {
	return std::ranges::any_of(m_tracks, [](Track const& t) { return t.status != Track::Status::Error; });
}

auto Tracklist::has_next_track() const -> bool {
	if (m_tracks.empty()) { return false; }
	if (is_inactive()) { return true; }
	if (!is_last(m_active)) { return true; }
	return false;
}

auto Tracklist::push(std::string_view const path) -> bool {
	auto const extension = fs::path{path}.extension().generic_string();
	switch (get_file_type(extension)) {
	case FileType::Music: append_track(path); return true;
	case FileType::Playlist: return append_playlist(path);
	default: return false;
	}
}

void Tracklist::clear() {
	m_tracks.clear();
	m_active = m_cursor = m_tracks.end();
}

auto Tracklist::save_playlist(std::string_view const path) const -> bool {
	if (m_tracks.empty() || path.empty()) { return false; }
	auto playlist = Playlist{};
	playlist.paths.reserve(m_tracks.size());
	for (auto const& track : m_tracks) { playlist.paths.push_back(track.path); }
	return playlist.save_to(path);
}

auto Tracklist::cycle_next() -> Track* {
	if (m_tracks.empty()) { return nullptr; }
	if (is_inactive() || is_last(m_active)) {
		m_active = m_tracks.begin();
	} else {
		++m_active;
	}
	return &*m_active;
}

auto Tracklist::cycle_prev() -> Track* {
	if (m_tracks.empty()) { return nullptr; }
	if (is_first(m_active)) { m_active = m_tracks.end(); }
	--m_active;
	return &*m_active;
}

void Tracklist::update() {
	ImGui::TextUnformatted(ICON_KI_LIST);
	auto const none_selected = m_cursor == m_tracks.end();
	if (none_selected) { ImGui::BeginDisabled(); }
	ImGui::SameLine();
	remove_track();
	ImGui::SameLine();
	move_track_up();
	ImGui::SameLine();
	move_track_down();
	if (none_selected) { ImGui::EndDisabled(); }
	auto const is_empty = m_tracks.empty();
	if (is_empty) { ImGui::BeginDisabled(); }
	ImGui::SameLine();
	if (ImGui::Button(ICON_KI_SAVE)) { m_events->save(); }
	if (is_empty) { ImGui::EndDisabled(); }
	track_list();
}

auto Tracklist::is_inactive() const -> bool { return m_active == m_tracks.end(); }

auto Tracklist::is_first(It const it) const -> bool { return it == m_tracks.begin(); }

auto Tracklist::is_last(It const it) const -> bool { return it != m_tracks.end() && std::next(it) == m_tracks.end(); }

auto Tracklist::append_playlist(std::string_view const path) -> bool {
	auto playlist = Playlist{};
	if (!playlist.append_from(path)) { return false; }
	for (auto const& path : playlist.paths) { append_track(path); }
	return true;
}

void Tracklist::append_track(std::string_view const path) {
	auto const fs_path = fs::absolute(path);
	m_tracks.push_back(to_track(fs_path, m_prev_id));
}

void Tracklist::remove_track() {
	if (ImGui::Button(ICON_KI_TIMES)) {
		if (m_active == m_cursor) {
			m_events->unload_active();
			m_active = m_tracks.end();
		}
		m_cursor = m_tracks.erase(m_cursor);
	}
}

void Tracklist::move_track_up() {
	auto const on_first_track = is_first(m_cursor);
	if (on_first_track) { ImGui::BeginDisabled(); }
	if (ImGui::Button(ICON_KI_ARROW_TOP)) { swap_with_cursor(std::prev(m_cursor)); }
	if (on_first_track) { ImGui::EndDisabled(); }
}

void Tracklist::move_track_down() {
	auto const on_last_track = !is_inactive() && is_last(m_cursor);
	if (on_last_track) { ImGui::BeginDisabled(); }
	if (ImGui::Button(ICON_KI_ARROW_BOTTOM)) { swap_with_cursor(std::next(m_cursor)); }
	if (on_last_track) { ImGui::EndDisabled(); }
}

void Tracklist::track_list() {
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
			imcpp::align_right(width);
			ImGui::TextUnformatted(track.duration_label.c_str());
		}
	}
	ImGui::EndChild();

	if (switch_track) {
		assert(m_cursor != m_tracks.end());
		m_active = m_cursor;
		m_events->play_track(&*m_active);
	}
}

void Tracklist::swap_with_cursor(It const& it) {
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

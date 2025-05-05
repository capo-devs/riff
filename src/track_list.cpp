#include <IconsKenney.h>
#include <imgui.h>
#include <track_list.hpp>
#include <util.hpp>
#include <algorithm>
#include <array>
#include <filesystem>
#include <ranges>
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
	return m_active + 1 < int(m_tracks.size());
}

auto Tracklist::push(char const* path) -> bool {
	auto const fs_path = fs::path{path};
	if (!is_music(fs_path)) { return false; }
	m_tracks.push_back(to_track(fs_path, m_prev_id));
	return true;
}

void Tracklist::clear() {
	m_tracks.clear();
	m_active = m_cursor = -1;
}

auto Tracklist::cycle_next() -> Track* {
	if (m_tracks.empty()) { return nullptr; }
	m_active = (m_active + 1) % int(m_tracks.size());
	return &m_tracks.at(std::size_t(m_active));
}

auto Tracklist::cycle_prev() -> Track* {
	if (m_tracks.empty()) { return nullptr; }
	if (m_active <= 0) {
		m_active = int(m_tracks.size()) - 1;
	} else {
		--m_active;
	}
	return &m_tracks.at(std::size_t(m_active));
}

void Tracklist::update(IMediator& mediator) {
	ImGui::TextUnformatted(ICON_KI_LIST);
	auto const none_selected = m_cursor < 0;
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

void Tracklist::remove_track(IMediator& mediator) {
	if (ImGui::Button(ICON_KI_TIMES)) {
		if (m_active == m_cursor) {
			mediator.unload_active();
			m_active = -1;
		}
		auto const index = std::ptrdiff_t(m_cursor);
		m_tracks.erase(m_tracks.begin() + index);
		if (m_active > index) { --m_active; }
		if (std::size_t(m_cursor) >= m_tracks.size()) { m_cursor = int(m_tracks.size() - 1); }
	}
}

void Tracklist::move_track_up() {
	auto const is_first = m_cursor == 0;
	if (is_first) { ImGui::BeginDisabled(); }
	if (ImGui::Button(ICON_KI_ARROW_TOP)) {
		assert(m_cursor > 0);
		swap_track_at_cursor(m_cursor - 1);
	}
	if (is_first) { ImGui::EndDisabled(); }
}

void Tracklist::move_track_down() {
	auto const is_last = !m_tracks.empty() && m_cursor == int(m_tracks.size() - 1);
	if (is_last) { ImGui::BeginDisabled(); }
	if (ImGui::Button(ICON_KI_ARROW_BOTTOM)) {
		assert(m_cursor + 1 < int(m_tracks.size()));
		swap_track_at_cursor(m_cursor + 1);
	}
	if (is_last) { ImGui::EndDisabled(); }
}

void Tracklist::track_list(IMediator& mediator) {
	auto switch_track = false;
	ImGui::BeginChild("Tracklist", {}, ImGuiChildFlags_Borders);
	for (auto const [index, track] : std::views::enumerate(m_tracks)) {
		auto const is_now_playing = m_active == int(index);
		auto const is_error = track.status == Track::Status::Error;
		if (is_error) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0f, 0.3f, 0.0f, 1.0f});
		} else if (is_now_playing) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.5f, 1.0f, 0.2f, 1.0f});
		}
		auto const is_selected = m_cursor == int(index);
		if (ImGui::Selectable(track.label.c_str(), is_selected)) { m_cursor = int(index); }
		if (is_now_playing || is_error) { ImGui::PopStyleColor(); }
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			m_cursor = int(index);
			switch_track = true;
		}
		if (track.status == Track::Status::Ok) {
			ImGui::SameLine();
			auto const width = ImGui::CalcTextSize(track.duration_label.c_str()).x;
			util::align_right(width);
			ImGui::TextUnformatted(track.duration_label.c_str());
		}
	}
	ImGui::EndChild();

	if (switch_track) {
		auto& track = m_tracks.at(std::size_t(m_cursor));
		m_active = mediator.play_track(track) ? m_cursor : -1;
	}
}

void Tracklist::swap_track_at_cursor(int const with) {
	if (m_cursor < 0 || with < 0) { return; }
	auto const idx_a = std::size_t(m_cursor);
	auto const idx_b = std::size_t(with);
	if (idx_a >= m_tracks.size() || idx_b >= m_tracks.size()) { return; }

	std::swap(m_tracks.at(idx_a), m_tracks.at(idx_b));
	if (m_active == m_cursor) {
		m_active = with;
	} else if (m_active == with) {
		m_active = m_cursor;
	}
	m_cursor = with;
}
} // namespace riff

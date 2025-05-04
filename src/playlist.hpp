#pragma once
#include <track.hpp>
#include <cstdint>
#include <span>
#include <vector>

namespace riff {
class Playlist {
  public:
	enum class Action : std::int8_t { None, Load, Unload };

	[[nodiscard]] auto get_tracks() const -> std::span<Track const> { return m_tracks; }
	[[nodiscard]] auto has_playable_track() const -> bool;

	[[nodiscard]] auto get_active() const -> Track const*;
	[[nodiscard]] auto get_active() -> Track*;

	auto push(char const* path) -> bool;
	auto remove_active() -> bool;
	void clear();

	auto cycle_next() -> Track*;
	auto cycle_prev() -> Track*;

	auto update() -> Action;

  private:
	void remove_track();
	void move_track_up();
	void move_track_down();
	void track_list();
	void swap_track_at_cursor(int with);

	std::vector<Track> m_tracks{};
	std::uint64_t m_prev_id{};
	int m_cursor{-1};
	int m_active{-1};

	Action m_action{Action::None};
};
} // namespace riff

#pragma once
#include <track.hpp>
#include <cstdint>
#include <span>
#include <variant>
#include <vector>

namespace riff {
class Tracklist {
  public:
	struct PlayTrack {
		int index{};
	};

	struct UnloadTrack {};

	using Action = std::variant<std::monostate, PlayTrack, UnloadTrack>;

	[[nodiscard]] auto get_tracks() const -> std::span<Track const> { return m_tracks; }
	[[nodiscard]] auto get_track(int index) -> Track*;
	[[nodiscard]] auto has_playable_track() const -> bool;
	[[nodiscard]] auto has_next_track() const -> bool;

	[[nodiscard]] auto get_active() const -> Track const*;
	[[nodiscard]] auto get_active() -> Track*;
	auto set_active(int index) -> bool;

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

	Action m_action{};
};
} // namespace riff

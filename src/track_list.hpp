#pragma once
#include <klib/base_types.hpp>
#include <track.hpp>
#include <cstdint>
#include <span>
#include <vector>

namespace riff {
class Tracklist {
  public:
	struct IMediator : klib::Polymorphic {
		virtual auto play_track(Track& track) -> bool = 0;
		virtual void unload_active() = 0;
	};

	[[nodiscard]] auto get_tracks() const -> std::span<Track const> { return m_tracks; }
	[[nodiscard]] auto has_playable_track() const -> bool;
	[[nodiscard]] auto has_next_track() const -> bool;

	auto push(char const* path) -> bool;
	void clear();

	auto cycle_next() -> Track*;
	auto cycle_prev() -> Track*;

	void update(IMediator& mediator);

  private:
	void remove_track(IMediator& mediator);
	void move_track_up();
	void move_track_down();
	void track_list(IMediator& mediator);
	void swap_track_at_cursor(int with);

	std::vector<Track> m_tracks{};
	std::uint64_t m_prev_id{};
	int m_cursor{-1};
	int m_active{-1};
};
} // namespace riff

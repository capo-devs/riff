#pragma once
#include <klib/base_types.hpp>
#include <track.hpp>
#include <cstdint>
#include <list>

namespace riff {
class Tracklist : public klib::Pinned {
  public:
	struct IMediator : klib::Polymorphic {
		virtual auto play_track(Track& track) -> bool = 0;
		virtual void unload_active() = 0;
	};

	[[nodiscard]] auto is_empty() const -> bool { return m_tracks.empty(); }
	[[nodiscard]] auto has_playable_track() const -> bool;
	[[nodiscard]] auto has_next_track() const -> bool;

	auto push(char const* path) -> bool;
	void clear();

	auto cycle_next() -> Track*;
	auto cycle_prev() -> Track*;

	void update(IMediator& mediator);

  private:
	using It = std::list<Track>::iterator;

	void remove_track(IMediator& mediator);
	void move_track_up();
	void move_track_down();
	void track_list(IMediator& mediator);
	void swap_with_cursor(It it);

	std::list<Track> m_tracks{};
	std::uint64_t m_prev_id{};
	It m_cursor{m_tracks.end()};
	It m_active{m_tracks.end()};
};
} // namespace riff

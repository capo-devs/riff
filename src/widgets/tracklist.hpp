#pragma once
#include <klib/base_types.hpp>
#include <klib/c_string.hpp>
#include <types/event.hpp>
#include <types/track.hpp>
#include <cstdint>
#include <gsl/pointers>
#include <list>

namespace riff {
class Tracklist : public klib::Pinned {
  public:
	explicit Tracklist(gsl::not_null<Events*> events) : m_events(events) {}

	[[nodiscard]] auto is_empty() const -> bool { return m_tracks.empty(); }
	[[nodiscard]] auto has_playable_track() const -> bool;
	[[nodiscard]] auto has_next_track() const -> bool;

	auto push(std::string_view path) -> bool;
	void clear();

	[[nodiscard]] auto save_playlist(std::string_view path) const -> bool;

	auto cycle_next() -> Track*;
	auto cycle_prev() -> Track*;

	void reset_active() { m_active = m_tracks.end(); }

	void update();

  private:
	using It = std::list<Track>::iterator;

	[[nodiscard]] auto is_inactive() const -> bool;
	[[nodiscard]] auto is_first() const -> bool;
	[[nodiscard]] auto is_last() const -> bool;

	auto append_playlist(std::string_view path) -> bool;
	void append_track(std::string_view path);

	void remove_track();
	void move_track_up();
	void move_track_down();
	void track_list();
	void swap_with_cursor(It const& it);

	gsl::not_null<Events*> m_events;

	std::list<Track> m_tracks{};
	std::uint64_t m_prev_id{};
	It m_cursor{m_tracks.end()};
	It m_active{m_tracks.end()};
};
} // namespace riff

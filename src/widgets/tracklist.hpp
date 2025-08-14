#pragma once
#include <klib/base_types.hpp>
#include <klib/c_string.hpp>
#include <types/track.hpp>
#include <cstdint>
#include <list>

namespace riff {
class Tracklist : public klib::Pinned {
  public:
	struct IMediator : klib::Polymorphic {
		virtual auto play_track(Track& track) -> bool = 0;
		virtual void unload_active() = 0;
		virtual void on_save() = 0;
	};

	[[nodiscard]] auto is_empty() const -> bool { return m_tracks.empty(); }
	[[nodiscard]] auto has_playable_track() const -> bool;
	[[nodiscard]] auto has_next_track() const -> bool;

	auto push(std::string_view path) -> bool;
	void clear();

	[[nodiscard]] auto save_playlist(std::string_view path) const -> bool;

	auto cycle_next() -> Track*;
	auto cycle_prev() -> Track*;

	void update(IMediator& mediator);

  private:
	using It = std::list<Track>::iterator;

	[[nodiscard]] auto is_inactive() const -> bool;
	[[nodiscard]] auto is_first() const -> bool;
	[[nodiscard]] auto is_last() const -> bool;

	auto append_playlist(std::string_view path) -> bool;
	void append_track(std::string_view path);

	void remove_track(IMediator& mediator);
	void move_track_up();
	void move_track_down();
	void track_list(IMediator& mediator);
	void swap_with_cursor(It const& it);

	std::list<Track> m_tracks{};
	std::uint64_t m_prev_id{};
	It m_cursor{m_tracks.end()};
	It m_active{m_tracks.end()};
};
} // namespace riff

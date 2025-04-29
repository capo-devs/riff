#pragma once
#include <state.hpp>
#include <string>

namespace riff {
class Frontend {
  public:
	explicit Frontend(State* state);

	void set_now_playing(Track const& track);

	void draw();

  private:
	void update_dynamic();

	void draw_menu_bar();
	void draw_controls(Track const* active_track);
	void draw_buttons(Track const* active_track);
	void draw_seekbar(Time track_duration);
	void draw_playlist();

	State* m_state{};

	std::string m_duration_str{};
	std::string m_cursor_str{};

	float m_fcursor{};
	int m_volume{};
	std::int32_t m_selected{-1};
	bool m_seeking{};
};
} // namespace riff

#pragma once
#include <capo/engine.hpp>
#include <track.hpp>

namespace riff {
class Player {
  public:
	enum class Action : std::int8_t { None, Previous, Next };

	explicit Player();

	[[nodiscard]] auto get_volume() const -> int { return int(m_source->get_gain() * 100.0f); }
	void set_volume(int const volume) { m_source->set_gain(float(volume) * 0.01f); }

	[[nodiscard]] auto get_balance() const -> float { return m_source->get_pan(); }
	void set_balance(float const balance) { m_source->set_pan(balance); }

	[[nodiscard]] auto get_cursor() const -> Time { return m_source->get_cursor(); }
	void set_cursor(Time cursor) { m_source->set_cursor(cursor); }

	[[nodiscard]] auto is_track_loaded() const -> bool { return m_source->is_bound(); }
	auto load_track(Track& track) -> bool;
	void unload_track() { m_source->unbind(); }

	[[nodiscard]] auto at_end() const -> bool { return m_source->at_end(); }
	[[nodiscard]] auto is_playing() const -> bool { return m_source->is_playing(); }
	void play() { m_source->play(); }
	void pause() { m_source->stop(); }

	auto update() -> Action;

  private:
	static constexpr std::string_view blank_title_v{"[none]"};

	void buttons();
	void sliders();
	void seekbar();

	std::unique_ptr<capo::IEngine> m_engine{};
	std::unique_ptr<capo::ISource> m_source{};

	std::string m_title{blank_title_v};
	std::string m_duration_str{};

	float m_cursor{};
	std::string m_cursor_str{};
	bool m_seeking{};

	Action m_action{Action::None};
};
} // namespace riff

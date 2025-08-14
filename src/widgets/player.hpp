#pragma once
#include <capo/source.hpp>
#include <klib/base_types.hpp>
#include <klib/c_string.hpp>
#include <types/repeat.hpp>
#include <types/track.hpp>

namespace riff {
class Player {
  public:
	struct IMediator : klib::Polymorphic {
		virtual void skip_prev() = 0;
		virtual void skip_next() = 0;
	};

	explicit Player(std::unique_ptr<capo::ISource> source);

	[[nodiscard]] auto get_volume() const -> int { return int(m_source->get_gain() * 100.0f); }
	void set_volume(int const volume) { m_source->set_gain(float(volume) * 0.01f); }

	[[nodiscard]] auto get_balance() const -> float { return m_source->get_pan(); }
	void set_balance(float const balance) { m_source->set_pan(balance); }

	[[nodiscard]] auto get_repeat() const -> Repeat { return m_repeat; }
	void set_repeat(Repeat repeat);

	[[nodiscard]] auto get_cursor() const -> Time { return m_source->get_cursor(); }
	void set_cursor(Time cursor) { m_source->set_cursor(cursor); }

	[[nodiscard]] auto is_track_loaded() const -> bool { return m_source->is_bound(); }
	auto load_track(Track& track) -> bool;
	void unload_track();

	[[nodiscard]] auto at_end() const -> bool { return m_source->at_end(); }
	[[nodiscard]] auto is_playing() const -> bool { return m_source->is_playing(); }
	void play() { m_source->play(); }
	void pause() { m_source->stop(); }

	void update(IMediator& mediator);

  private:
	static constexpr std::string_view blank_title_v{"[none]"};

	void buttons(IMediator& mediator);
	void sliders();
	void seekbar();

	std::unique_ptr<capo::ISource> m_source{};

	klib::CString m_title{blank_title_v.data()};
	klib::CString m_duration_str{};

	float m_cursor{};
	std::string m_cursor_str{};
	bool m_seeking{};

	Repeat m_repeat{Repeat::None};
};
} // namespace riff

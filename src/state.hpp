#pragma once
#include <track.hpp>
#include <functional>
#include <vector>

namespace riff {
class Signal {
  public:
	using Callback = std::move_only_function<void()>;

	void connect(Callback callback) {
		if (!callback) { return; }
		m_callbacks.push_back(std::move(callback));
	}

	void dispatch() {
		for (auto& callback : m_callbacks) { callback(); }
	}

  private:
	std::vector<Callback> m_callbacks{};
};

struct State {
	[[nodiscard]] auto active_track() const -> Track const* {
		return now_playing >= 0 ? &playlist.at(std::size_t(now_playing)) : nullptr;
	}

	std::vector<Track> playlist{};
	std::int32_t now_playing{-1};

	float gain{1.0f};
	float balance{0.0f};
	bool is_playing{false};
	Time cursor{};

	Signal track_changed{};
	Signal gain_changed{};
	Signal balance_changed{};
	Signal on_play{};
	Signal on_pause{};
	Signal on_seek{};
	Signal on_quit{};
	Signal on_track_select{};
	Signal on_unbind{};
	Signal on_play_next{};
	Signal on_play_previous{};
};
} // namespace riff

#pragma once
#include <track.hpp>
#include <functional>
#include <optional>
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
		return now_playing ? &playlist.at(*now_playing) : nullptr;
	}

	std::vector<Track> playlist{};
	std::optional<std::size_t> now_playing{};

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
};
} // namespace riff

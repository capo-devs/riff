#include <backend.hpp>
#include <cassert>
#include <filesystem>

namespace riff {
namespace fs = std::filesystem;

Backend::Backend(State* state) : m_state(state) {
	assert(m_state);

	m_engine = capo::create_engine();
	if (!m_engine) { throw std::runtime_error{"Failed to create Audio Engine"}; }
	m_source = m_engine->create_source();
	m_checker = m_engine->create_source();
	if (!m_source || !m_checker) { throw std::runtime_error{"Failed to create Audio Source"}; }

	auto const set_gain = [this] { m_source->set_gain(m_state->gain); };
	auto const set_pan = [this] { m_source->set_pan(m_state->balance); };

	m_state->on_seek.connect([this] { m_source->set_cursor(m_state->cursor); });
	m_state->on_play.connect([this] {
		m_source->play();
		m_state->is_playing = true;
	});
	m_state->on_pause.connect([this] {
		m_source->stop();
		m_state->is_playing = false;
	});
	m_state->on_unbind.connect([this] {
		m_state->now_playing = -1;
		m_state->is_playing = false;
		m_source->unbind();
	});
	m_state->on_play_next.connect([this] { play_next(true); });
	m_state->on_play_previous.connect([this] { play_previous(); });
	m_state->on_track_select.connect([this] { play_next(false); });
	m_state->gain_changed.connect([set_gain] { set_gain(); });
	m_state->balance_changed.connect([set_pan] { set_pan(); });

	set_gain();
	set_pan();
}

void Backend::on_drop(std::span<char const* const> paths) {
	auto const was_empty = m_state->playlist.empty();
	for (auto const* path : paths) {
		auto track = Track{.path = path};
		if (!check_track(track)) { continue; }
		m_state->playlist.push_back(std::move(track));
	}
	if (was_empty && !m_state->playlist.empty()) { play_next(true); }
	return;

	auto const* path = paths.front();
	if (!m_source->open_stream(path)) {
		// TODO: error
		return;
	}

	m_state->playlist = {Track{
		.path = path,
		.name = fs::path{path}.filename().generic_string(),
		.duration = m_source->get_duration(),
		.status = Track::Status::Ok,
	}};
	m_state->now_playing = 0;
	m_state->track_changed.dispatch();
	m_source->play();
}

void Backend::update() {
	auto const track_ended = m_state->is_playing && !m_source->is_playing();
	if (track_ended) { play_next(true); }
	if (m_source->is_bound()) { m_state->cursor = m_source->get_cursor(); }
	m_state->gain = m_source->get_gain();
	m_state->is_playing = m_source->is_playing();
}

auto Backend::check_track(Track& track) -> bool {
	if (!m_checker->open_stream(track.path.c_str())) {
		track.status = Track::Status::Error;
		return false;
	}
	track.name = fs::path{track.path}.filename().generic_string();
	track.status = Track::Status::Ok;
	track.duration = m_checker->get_duration();
	m_checker->unbind();
	return true;
}

void Backend::play_next(bool const pre_increment) {
	if (m_state->now_playing < 0) {
		m_state->now_playing = 0;
	} else if (pre_increment) {
		++m_state->now_playing;
	}
	auto const pred = [this] { return m_state->now_playing < std::int32_t(m_state->playlist.size()); };
	auto const post = [this] { ++m_state->now_playing; };
	try_play(pred, post);
}

void Backend::play_previous() {
	if (m_state->cursor > 3s) {
		m_state->cursor = 0s;
		m_state->on_seek.dispatch();
		return;
	}

	--m_state->now_playing;
	auto const pred = [this] { return m_state->now_playing >= 0; };
	auto const post = [this] { --m_state->now_playing; };
	try_play(pred, post);
}

template <typename Pred, typename Post>
void Backend::try_play(Pred pred, Post post) {
	if (m_state->now_playing == -1) { return; }

	for (; pred(); post()) {
		auto& track = m_state->playlist.at(std::size_t(m_state->now_playing));
		if (play_track(track)) {
			m_state->track_changed.dispatch();
			return;
		}
	}

	m_state->is_playing = false;
	m_state->now_playing = -1;
	m_state->track_changed.dispatch();
}

auto Backend::play_track(Track& track) -> bool {
	if (!m_source->open_stream(track.path.c_str())) {
		track.status = Track::Status::Error;
		return false;
	}
	m_state->on_play.dispatch();
	m_state->is_playing = true;
	return true;
}
} // namespace riff

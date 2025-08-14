#include <input/controller.hpp>
#include <cassert>

namespace riff::input {
Controller::Controller(gsl::not_null<Events*> events) : m_events(events) { add_bindings(); }

void Controller::on_key(int const key, int const action, int const mods) {
	auto const actor = key;
	m_key_mapping.dispatch(actor, action, mods);
}

void Controller::add_bindings() {
	m_key_mapping.bind(GLFW_KEY_SPACE, Action::Press, [this](Modifier const mods) {
		if (mods != Modifier::None) { return; }
		m_events->toggle_playback();
	});

	m_key_mapping.bind(GLFW_KEY_LEFT, Action::Press | Action::Repeat,
					   [this](Modifier const mods) { m_events->seek(Polarity::Negative, mods); });
	m_key_mapping.bind(GLFW_KEY_RIGHT, Action::Press | Action::Repeat,
					   [this](Modifier const mods) { m_events->seek(Polarity::Positive, mods); });

	m_key_mapping.bind(GLFW_KEY_R, Action::Press, [this](Modifier const mods) {
		if (mods != Modifier::None) { return; }
		m_events->toggle_repeat();
	});

	m_key_mapping.bind(GLFW_KEY_P, Action::Press, [this](Modifier const mods) {
		if (mods != Modifier::None) { return; }
		m_events->skip(Polarity::Negative);
	});
	m_key_mapping.bind(GLFW_KEY_N, Action::Press, [this](Modifier const mods) {
		if (mods != Modifier::None) { return; }
		m_events->skip(Polarity::Positive);
	});

	m_key_mapping.bind(GLFW_KEY_DOWN, Action::Press | Action::Repeat,
					   [this](Modifier const mods) { m_events->adjust_volume(Polarity::Negative, mods); });
	m_key_mapping.bind(GLFW_KEY_UP, Action::Press | Action::Repeat,
					   [this](Modifier const mods) { m_events->adjust_volume(Polarity::Positive, mods); });
}
} // namespace riff::input

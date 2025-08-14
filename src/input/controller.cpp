#include <input/controller.hpp>
#include <cassert>

namespace riff::input {
Controller::Controller(gsl::not_null<Events*> events) : m_events(events) { add_bindings(); }

void Controller::on_key(int const key, int const action, int const mods) {
	auto const actor = key;
	m_key_mapping.dispatch(actor, action, mods);
}

void Controller::add_bindings() {
	m_key_mapping.bind(GLFW_KEY_SPACE, Action::Press, [this](int mods) {
		if (mods != 0) { return; }
		m_events->toggle_playback();
	});
}
} // namespace riff::input

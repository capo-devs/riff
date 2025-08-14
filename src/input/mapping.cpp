#include <input/mapping.hpp>

namespace riff::input {
namespace {
constexpr auto to_action(int const glfw_action) {
	switch (glfw_action) {
	case GLFW_PRESS: return Action::Press;
	case GLFW_REPEAT: return Action::Repeat;
	case GLFW_RELEASE: return Action::Release;
	default: return Action::None;
	}
}
} // namespace

void Mapping::bind(int const actor, Action const actions, Callback callback) {
	if (!callback || actions == Action::None) { return; }
	m_bindings.insert_or_assign(actor, Binding{.actions = actions, .callback = std::move(callback)});
}

void Mapping::dispatch(int const actor, int const glfw_action, int const mods) {
	auto const it = m_bindings.find(actor);
	if (it == m_bindings.end()) { return; }
	auto& binding = it->second;
	if (!binding.callback) { return; }
	auto const action = to_action(glfw_action);
	if ((action & binding.actions) == Action::None) { return; }
	binding.callback(mods);
}
} // namespace riff::input

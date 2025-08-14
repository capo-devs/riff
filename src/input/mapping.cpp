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

constexpr auto to_mods(int const glfw_mods) {
	auto ret = Modifier{};
	if (glfw_mods & GLFW_MOD_SHIFT) { ret |= Modifier::Shift; }
	if (glfw_mods & GLFW_MOD_CONTROL) { ret |= Modifier::Control; }
	if (glfw_mods & GLFW_MOD_ALT) { ret |= Modifier::Alt; }
	return ret;
}
} // namespace

void Mapping::bind(int const actor, Action const actions, Callback callback) {
	if (!callback || actions == Action::None) { return; }
	m_bindings.insert_or_assign(actor, Binding{.actions = actions, .callback = std::move(callback)});
}

void Mapping::dispatch(int const actor, int const glfw_action, int const glfw_mods) {
	auto const it = m_bindings.find(actor);
	if (it == m_bindings.end()) { return; }
	auto& binding = it->second;
	if (!binding.callback) { return; }
	auto const action = to_action(glfw_action);
	if ((action & binding.actions) == Action::None) { return; }
	binding.callback(to_mods(glfw_mods));
}
} // namespace riff::input

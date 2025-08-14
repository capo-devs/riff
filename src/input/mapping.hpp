#pragma once
#include <GLFW/glfw3.h>
#include <input/action.hpp>
#include <input/modifier.hpp>
#include <klib/enum_ops.hpp>
#include <functional>
#include <unordered_map>

namespace riff::input {
class Mapping {
  public:
	using Callback = std::move_only_function<void(Modifier mods)>;

	void bind(int actor, Action actions, Callback callback);

	void dispatch(int actor, int glfw_action, int glfw_mods);

  private:
	struct Binding {
		Action actions{};
		Callback callback{};
	};

	std::unordered_map<int, Binding> m_bindings{};
};
} // namespace riff::input

#pragma once
#include <GLFW/glfw3.h>
#include <klib/enum_ops.hpp>
#include <cstdint>
#include <functional>
#include <unordered_map>

namespace riff::input {
enum class Action : std::uint8_t;
} // namespace riff::input

template <>
inline constexpr auto klib::enable_enum_ops_v<riff::input::Action> = true;

namespace riff::input {
enum class Action : std::uint8_t {
	None = 0,
	Press = 1 << 0,
	Repeat = 1 << 1,
	Release = 1 << 2,
};

class Mapping {
  public:
	using Callback = std::move_only_function<void(int mods)>;

	void bind(int actor, Action actions, Callback callback);

	void dispatch(int actor, int glfw_action, int mods);

  private:
	struct Binding {
		Action actions{};
		Callback callback{};
	};

	std::unordered_map<int, Binding> m_bindings{};
};
} // namespace riff::input

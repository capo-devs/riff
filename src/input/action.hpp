#pragma once
#include <klib/enum_ops.hpp>
#include <cstdint>

namespace riff::input {
enum class Action : std::uint8_t {
	None = 0,
	Press = 1 << 0,
	Repeat = 1 << 1,
	Release = 1 << 2,
};
} // namespace riff::input

template <>
inline constexpr auto klib::enable_enum_ops_v<riff::input::Action> = true;

#pragma once
#include <klib/enum_ops.hpp>
#include <cstdint>

namespace riff::input {
enum class Modifier : std::uint8_t {
	None = 0,
	Shift = 1 << 0,
	Control = 1 << 1,
	Alt = 1 << 2,
};
} // namespace riff::input

template <>
inline constexpr auto klib::enable_enum_ops_v<riff::input::Modifier> = true;

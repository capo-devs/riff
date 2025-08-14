#pragma once
#include <cstdint>

namespace riff {
enum class Polarity : std::int8_t { Negative, Positive };

[[nodiscard]] constexpr auto to_f32(Polarity const polarity) -> float {
	return polarity == Polarity::Negative ? -1.0f : 1.0f;
}

[[nodiscard]] constexpr auto to_int(Polarity const polarity) -> int {
	return polarity == Polarity::Negative ? -1.0f : 1.0f;
}
} // namespace riff

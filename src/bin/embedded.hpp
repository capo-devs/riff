#pragma once
#include <cstddef>
#include <span>

namespace riff {
[[nodiscard]] auto kenny_icon_bytes() -> std::span<std::byte const>;
[[nodiscard]] auto rounded_elegance_bytes() -> std::span<std::byte const>;
} // namespace riff

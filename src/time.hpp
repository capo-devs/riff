#pragma once
#include <chrono>

namespace riff {
using namespace std::chrono_literals;

using Clock = std::chrono::steady_clock;

using Time = std::chrono::duration<float>;
} // namespace riff

#pragma once
#include <time.hpp>
#include <cstdint>
#include <string>

namespace riff {
struct Track {
	enum class Status : std::int8_t { None, Error, Ok };

	std::string path{};
	std::string name{};
	std::string label{};
	std::string duration_label{};
	Time duration{};
	Status status{Status::None};
};
} // namespace riff

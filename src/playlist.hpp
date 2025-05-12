#pragma once
#include <string>
#include <vector>

namespace riff {
struct Playlist {
	std::vector<std::string> paths{};

	auto append_from(std::string_view path) -> bool;
	[[nodiscard]] auto save_to(std::string_view path) const -> bool;
};
} // namespace riff

#include <playlist.hpp>
#include <filesystem>
#include <fstream>

namespace riff {
namespace fs = std::filesystem;

namespace {
auto ensure_dir_exists(fs::path const& path) {
	if (path.empty() || fs::is_directory(path)) { return true; }
	auto err = std::error_code{};
	return fs::create_directories(path.parent_path(), err);
}
} // namespace

auto Playlist::append_from(std::string_view const path) -> bool {
	auto const path_ = fs::absolute(path);
	if (!fs::is_regular_file(path_)) { return false; }
	auto file = std::ifstream{path_};
	if (!file.is_open()) { return false; }
	for (auto line = std::string{}; std::getline(file, line);) {
		if (line.starts_with('#')) { continue; }
		paths.push_back(std::move(line));
	}
	return true;
}

auto Playlist::save_to(std::string_view const path) const -> bool {
	if (paths.empty()) { return false; }
	auto const path_ = fs::absolute(path);
	if (!ensure_dir_exists(path_.parent_path())) { return false; }
	auto file = std::ofstream{path_};
	if (!file.is_open()) { return false; }
	for (auto const& path : paths) { file << path << '\n'; }
	return file.good();
}
} // namespace riff

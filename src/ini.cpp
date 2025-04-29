#include <ini.hpp>
#include <algorithm>
#include <format>
#include <fstream>

namespace riff {
namespace {
struct KeyValue {
	std::string_view key{};
	std::string_view value{};
};

constexpr auto trim_front(std::string_view in) {
	while (!in.empty() && std::isspace(static_cast<unsigned char>(in.front())) != 0) { in = in.substr(1); }
	return in;
}

constexpr auto trim_back(std::string_view in) {
	while (!in.empty() && std::isspace(static_cast<unsigned char>(in.back())) != 0) {
		in = in.substr(0, in.size() - 1);
	}
	return in;
}

constexpr auto create_key_value(std::string_view line) {
	auto ret = KeyValue{};
	auto const eq = line.find('=');
	if (eq == std::string_view::npos) { return ret; }
	ret.key = trim_back(line.substr(0, eq));
	ret.value = trim_front(trim_back(line.substr(eq + 1)));
	return ret;
}
} // namespace

auto Ini::load(char const* path) -> bool {
	auto file = std::ifstream{path};
	if (!file) { return false; }

	auto line = std::string{};
	while (std::getline(file, line)) {
		if (line.empty()) { continue; }
		parse_line(line);
	}
	return true;
}

auto Ini::get_value(std::string_view const key, std::string_view const fallback) const -> std::string_view {
	auto const it = m_map.find(key);
	if (it == m_map.end()) { return fallback; }
	return it->second;
}

auto Ini::serialize() const -> std::vector<std::string> {
	auto ret = std::vector<std::string>{};
	ret.reserve(m_map.size());
	for (auto const& [key, value] : m_map) { ret.push_back(std::format("{} = {}", key, value)); }
	std::ranges::sort(ret);
	return ret;
}

void Ini::parse_line(std::string_view line) {
	line = trim_front(line);
	if (line.empty() || line.starts_with('#')) { return; }

	auto const kv = create_key_value(line);
	if (kv.key.empty() || kv.value.empty()) { return; }

	m_map.insert_or_assign(std::string{kv.key}, std::string{kv.value});
}
} // namespace riff

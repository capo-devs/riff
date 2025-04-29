#pragma once
#include <klib/concepts.hpp>
#include <charconv>
#include <string>
#include <unordered_map>
#include <vector>

namespace riff {
class Ini {
  public:
	[[nodiscard]] auto load(char const* path) -> bool;

	[[nodiscard]] auto get_value(std::string_view key, std::string_view fallback = {}) const -> std::string_view;

	auto assign_to(std::string& out, std::string_view const key) const -> bool {
		auto const value = get_value(key);
		if (value.empty()) { return false; }
		out = value;
		return true;
	}

	template <klib::NumberT Type>
	auto assign_to(Type& out, std::string_view const key) const -> bool {
		auto const value = get_value(key);
		if (value.empty()) { return false; }
		auto const* end = value.data() + value.size();
		auto const [ptr, ec] = std::from_chars(value.data(), end, out);
		return ec == std::errc{} && ptr == end;
	}

	void set_value(std::string key, std::string value) { m_map.insert_or_assign(std::move(key), std::move(value)); }

	[[nodiscard]] auto serialize() const -> std::vector<std::string>;

  private:
	struct Hash : std::hash<std::string_view> {
		using is_transparent = void;
	};

	void parse_line(std::string_view line);

	std::unordered_map<std::string, std::string, Hash, std::equal_to<>> m_map{};
};
} // namespace riff

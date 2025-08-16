#include <core/config.hpp>
#include <core/ini.hpp>
#include <core/log.hpp>
#include <klib/enum_array.hpp>
#include <cmath>
#include <format>

namespace riff {
namespace {
constexpr auto repeat_str_v = klib::EnumArray<Repeat, std::string_view>{"none", "one", "all"};

constexpr void from_str(std::string_view const in, Repeat& out) {
	for (auto r = Repeat{}; r < Repeat::COUNT_; r = Repeat(int(r) + 1)) {
		if (in == repeat_str_v[r]) {
			out = r;
			return;
		}
	}
}

template <typename T, typename U, float Epsilon = 0.1f>
constexpr auto compare_eq(T const& a, U&& b) { // NOLINT(cppcoreguidelines-missing-std-forward)
	if constexpr (std::floating_point<T>) {
		return std::abs(a - T(b)) < T(Epsilon);
	} else {
		return a == b;
	}
}
} // namespace

auto Config::load() -> bool {
	if (load_silent()) {
		log.info("loaded config from: {}", path);
		return true;
	}
	log.warn("failed to load config from: {}", path);
	return false;
}

auto Config::save() const -> bool {
	if (save_silent()) {
		log.info("config saved to: {}", path);
		return true;
	}
	log.warn("failed to save config to: {}", path);
	return false;
}

auto Config::load_or_create() -> bool {
	if (load_silent()) {
		log.info("loaded config from: {}", path);
		return true;
	}

	if (save_silent()) {
		log.info("created config at: {}", path);
		return true;
	}

	log.warn("failed to create config at: {}", path);
	return false;
}

void Config::set_volume(int const volume) { set_if_different(m_volume, volume); }

void Config::set_balance(float const balance) { set_if_different(m_balance, balance); }

void Config::set_repeat(Repeat const repeat) { set_if_different(m_repeat, repeat); }

void Config::set_autosave(bool const autosave) { set_if_different(m_autosave, autosave); }

void Config::set_autosave_path(std::string_view const path) { set_if_different(m_autosave_path, path); }

void Config::update() {
	if (!m_dirty) { return; }
	auto const now = Clock::now();
	if (now - m_last_save < save_debounce_v) { return; }
	save();
}

auto Config::load_silent() -> bool {
	auto ini = Ini{};
	if (!ini.load(path.c_str())) { return false; }
	ini.assign_to(m_volume, "volume");
	ini.assign_to(m_balance, "balance");
	auto repeat_ = std::string{};
	if (ini.assign_to(repeat_, "repeat")) { from_str(repeat_, m_repeat); }
	ini.assign_to(m_autosave, "autosave");
	ini.assign_to(m_autosave_path, "autosave_path");
	m_dirty = false;
	return true;
}

auto Config::save_silent() const -> bool {
	auto ini = Ini{};
	ini.set_value("volume", std::format("{}", m_volume));
	ini.set_value("balance", std::format("{:.1f}", m_balance));
	ini.set_value("repeat", std::string{repeat_str_v[m_repeat]});
	ini.set_value("autosave", std::format("{}", m_autosave));
	ini.set_value("autosave_path", m_autosave_path);
	if (!ini.save(path.c_str())) { return false; }
	m_dirty = false;
	m_last_save = Clock::now();
	return true;
}

template <typename T, typename U>
void Config::set_if_different(T& out, U&& value) { // NOLINT(cppcoreguidelines-missing-std-forward)
	if (compare_eq(out, value)) { return; }
	out = value;
	m_dirty = true;
}
} // namespace riff

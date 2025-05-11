#include <config.hpp>
#include <ini.hpp>
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
} // namespace

auto Config::load() -> bool {
	auto ini = Ini{};
	if (!ini.load(path_v)) { return false; }
	ini.assign_to(m_volume, "volume");
	ini.assign_to(m_balance, "balance");
	auto repeat_ = std::string{};
	if (ini.assign_to(repeat_, "repeat")) { from_str(repeat_, m_repeat); }
	m_dirty = false;
	return true;
}

auto Config::save() const -> bool {
	auto ini = Ini{};
	ini.set_value("volume", std::format("{}", m_volume));
	ini.set_value("balance", std::format("{:.1f}", m_balance));
	ini.set_value("repeat", std::string{repeat_str_v[m_repeat]});
	if (!ini.save(path_v)) { return false; }
	m_dirty = false;
	m_last_save = Clock::now();
	return true;
}

void Config::set_volume(int const volume) {
	if (volume == m_volume) { return; }
	m_volume = volume;
	m_dirty = true;
}

void Config::set_balance(float const balance) {
	if (std::abs(balance - m_balance) < 0.01f) { return; }
	m_balance = balance;
	m_dirty = true;
}

void Config::set_repeat(Repeat const repeat) {
	if (repeat == m_repeat) { return; }
	m_repeat = repeat;
	m_dirty = true;
}

void Config::update() {
	if (!m_dirty) { return; }
	auto const now = Clock::now();
	if (now - m_last_save < save_debounce_v) { return; }
	save();
}
} // namespace riff

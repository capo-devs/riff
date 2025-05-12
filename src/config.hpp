#pragma once
#include <klib/c_string.hpp>
#include <repeat.hpp>
#include <time.hpp>

namespace riff {
class Config {
  public:
	static constexpr auto save_debounce_v{1s};

	Config(Config const&) = delete;
	Config(Config&&) = delete;
	auto operator=(Config const&) = delete;
	auto operator=(Config&&) = delete;

	Config() = default;
	~Config() { save(); }

	auto load() -> bool;
	auto save() const -> bool;
	auto load_or_create() -> bool;

	[[nodiscard]] auto get_volume() const -> int { return m_volume; }
	void set_volume(int volume);

	[[nodiscard]] auto get_balance() const -> float { return m_balance; }
	void set_balance(float balance);

	[[nodiscard]] auto get_repeat() const -> Repeat { return m_repeat; }
	void set_repeat(Repeat repeat);

	void update();

	std::string path{"riff.conf"};

  private:
	auto load_silent() -> bool;
	auto save_silent() const -> bool;

	int m_volume{100};
	float m_balance{0.0f};
	Repeat m_repeat{Repeat::None};

	mutable bool m_dirty{};
	mutable Clock::time_point m_last_save{};
};
} // namespace riff

#pragma once
#include <core/time.hpp>
#include <klib/c_string.hpp>
#include <types/repeat.hpp>

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

	[[nodiscard]] auto get_autosave() const -> bool { return m_autosave; }
	void set_autosave(bool autosave);

	[[nodiscard]] auto get_autosave_path() const -> std::string_view { return m_autosave_path; }
	void set_autosave_path(std::string_view path);

	void update();

	std::string path{"riff.conf"};

  private:
	auto load_silent() -> bool;
	auto save_silent() const -> bool;

	template <typename T, typename U>
	void set_if_different(T& out, U&& value);

	int m_volume{100};
	float m_balance{0.0f};
	Repeat m_repeat{Repeat::None};
	bool m_autosave{true};
	std::string m_autosave_path{"riff.m3u"};

	mutable bool m_dirty{};
	mutable Clock::time_point m_last_save{};
};
} // namespace riff

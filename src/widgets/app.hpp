#pragma once
#include <capo/engine.hpp>
#include <core/config.hpp>
#include <gvdi/app.hpp>
#include <widgets/imcpp.hpp>
#include <widgets/player.hpp>
#include <widgets/tracklist.hpp>

namespace riff {
struct Params {
	std::string_view config_path{"riff.conf"};
};

class App : public gvdi::App {
  public:
	explicit App(Params const& params) : m_params(params) {}

  private:
	struct SavePlaylist {
		static constexpr auto label_v = klib::CString{"Save Playlist"};

		auto update() -> bool;

		imcpp::InputText path{};
	};

	void pre_init() final;
	auto create_window() -> GLFWwindow* final;
	void post_init() final;
	void update() final;
	void create_engine();
	void create_player();
	void bind_events();

	void on_drop(std::span<char const* const> paths);
	void on_key(int key, int action, int mods);
	void update_config();

	auto play_track(Track& track) -> bool;
	void unload_active();
	void skip_prev();
	void skip_next();

	void save_playlist(std::string_view path);

	template <typename F>
	auto cycle(F get_track) -> bool;
	template <typename Pred, typename F>
	auto cycle(Pred pred, F get_track) -> bool;

	void advance();

	auto load_track(Track& track) -> bool;

	static void install_callbacks(GLFWwindow* window);

	Params m_params{};
	Config m_config{};
	Events m_events{};
	std::unique_ptr<capo::IEngine> m_engine{};
	std::optional<Player> m_player{};
	std::optional<Tracklist> m_tracklist{};

	bool m_playing{};
	SavePlaylist m_save_playlist{};
};
} // namespace riff

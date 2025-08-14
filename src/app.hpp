#pragma once
#include <capo/engine.hpp>
#include <config.hpp>
#include <gvdi/app.hpp>
#include <imcpp.hpp>
#include <player.hpp>
#include <tracklist.hpp>

namespace riff {
struct Params {
	std::string_view config_path{"riff.conf"};
};

class App : public gvdi::App, public Tracklist::IMediator, public Player::IMediator {
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

	auto play_track(Track& track) -> bool final;
	void unload_active() final;

	void skip_prev() final;
	void skip_next() final;
	void on_save() final;

	void create_engine();
	void create_player();

	void on_drop(std::span<char const* const> paths);
	void update_config();

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
	std::unique_ptr<capo::IEngine> m_engine{};
	std::optional<Player> m_player{};

	Tracklist m_tracklist{};
	bool m_playing{};
	SavePlaylist m_save_playlist{};
};
} // namespace riff

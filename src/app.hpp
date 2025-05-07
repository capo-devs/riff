#pragma once
#include <capo/engine.hpp>
#include <config.hpp>
#include <gvdi/context.hpp>
#include <player.hpp>
#include <tracklist.hpp>
#include <optional>

namespace riff {
class App : public Tracklist::IMediator, public Player::IMediator {
  public:
	void run();

  private:
	auto play_track(Track& track) -> bool final;
	void unload_active() final;

	void skip_prev() final;
	void skip_next() final;

	void create_engine();
	void create_player();
	void create_context();
	void setup_imgui();

	void on_drop(std::span<char const* const> paths);
	void update();
	void update_config();

	template <typename F>
	auto cycle(F get_track) -> bool;
	template <typename Pred, typename F>
	auto cycle(Pred pred, F get_track) -> bool;

	void advance();

	auto load_track(Track& track) -> bool;

	static void install_callbacks(GLFWwindow* window);

	Config m_config{};
	std::unique_ptr<capo::IEngine> m_engine{};
	std::optional<Player> m_player{};
	std::optional<gvdi::Context> m_context{};

	Tracklist m_tracklist{};
	bool m_playing{};
};
} // namespace riff

#pragma once
#include <capo/engine.hpp>
#include <gvdi/context.hpp>
#include <player.hpp>
#include <playlist.hpp>
#include <optional>

namespace riff {
class App {
  public:
	void run();

  private:
	void create_engine();
	void create_player();
	void create_context();
	void setup_imgui();

	void on_drop(std::span<char const* const> paths);
	void update();
	void update_player();
	void update_playlist();

	template <typename F>
	auto cycle(F get_track) -> bool;
	void on_next();
	void on_prev();

	static void install_callbacks(GLFWwindow* window);

	std::unique_ptr<capo::IEngine> m_engine{};
	std::optional<Player> m_player{};
	std::optional<gvdi::Context> m_context{};

	Playlist m_playlist{};
	bool m_playing{};
};
} // namespace riff

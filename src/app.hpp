#pragma once
#include <capo/engine.hpp>
#include <gvdi/context.hpp>
#include <player.hpp>
#include <playlist.hpp>
#include <optional>

namespace riff {
class App {
  public:
	auto run(int argc, char const* const* argv) -> int;

  private:
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

	std::optional<gvdi::Context> m_context{};

	std::optional<Player> m_player{};
	Playlist m_playlist{};
	bool m_playing{};
};
} // namespace riff

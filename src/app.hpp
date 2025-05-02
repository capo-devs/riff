#pragma once
#include <backend.hpp>
#include <capo/engine.hpp>
#include <frontend.hpp>
#include <gvdi/context.hpp>
#include <optional>

namespace riff {
class App {
  public:
	auto run(int argc, char const* const* argv) -> int;

  private:
	void create_context();
	void setup_imgui();

	void draw_frontend();

	static void install_callbacks(GLFWwindow* window);

	std::optional<gvdi::Context> m_context{};

	State m_state{};

	std::optional<Backend> m_backend{};
	std::optional<Frontend> m_frontend{};
};
} // namespace riff

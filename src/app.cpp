#include <app.hpp>
#include <build_version.hpp>
#include <capo/format.hpp>
#include <klib/args/parse.hpp>
#include <klib/version_str.hpp>
#include <log.hpp>
#include <cstdlib>
#include <print>

namespace riff {
namespace {
[[nodiscard]] auto self(GLFWwindow* window) -> App& { return *static_cast<App*>(glfwGetWindowUserPointer(window)); }
} // namespace

auto App::run(int const argc, char const* const* argv) -> int {
	auto const version = klib::to_string(build_version_v);
	auto const app_info = klib::args::ParseInfo{
		.help_text = "riff: audio player demo using capo-lite",
		.version = version,
	};
	auto const parse_result = klib::args::parse_main(app_info, {}, argc, argv);
	if (parse_result.early_return()) { return parse_result.get_return_code(); }

	m_state.on_quit.connect([this] { glfwSetWindowShouldClose(m_context->get_window(), GLFW_TRUE); });

	m_backend.emplace(&m_state);
	create_context();
	m_frontend.emplace(&m_state);

	while (m_context->next_frame()) {
		m_backend->update();
		draw_frontend();

		m_context->render();
	}

	return EXIT_SUCCESS;
}

void App::create_context() {
	auto window = gvdi::Context::create_window({500.0f, 250.0f}, "riff");
	if (!window) { throw std::runtime_error{"Failed to create Window"}; }

	glfwSetWindowUserPointer(window.get(), this);
	install_callbacks(window.get());

	m_context.emplace(std::move(window));
}

void App::draw_frontend() {
	auto const& viewport = *ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport.WorkPos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(viewport.WorkSize);
	static constexpr auto flags_v =
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
	if (ImGui::Begin("main", nullptr, flags_v)) { m_frontend->draw(); }
	ImGui::End();
}

void App::install_callbacks(GLFWwindow* window) {
	glfwSetDropCallback(window, [](GLFWwindow* window, int count, char const** paths) {
		self(window).m_backend->on_drop(std::span{paths, std::size_t(count)});
	});
}
} // namespace riff

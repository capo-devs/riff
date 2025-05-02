#include <IconsKenney.h>
#include <app.hpp>
#include <build_version.hpp>
#include <embedded.hpp>
#include <klib/args/parse.hpp>
#include <klib/version_str.hpp>
#include <log.hpp>
#include <cstdlib>

namespace riff {
namespace {
[[nodiscard]] auto self(GLFWwindow* window) -> App& { return *static_cast<App*>(glfwGetWindowUserPointer(window)); }

struct ImFontLoader {
	auto load(std::span<std::byte const> bytes, float const size, ImWchar const* glyph_ranges = {}) {
		auto config = ImFontConfig{};
		config.FontDataOwnedByAtlas = false;
		config.MergeMode = m_merge;
		m_merge = true;
		if (glyph_ranges == nullptr) { glyph_ranges = io.Fonts->GetGlyphRangesDefault(); }
		auto* data = const_cast<std::byte*>(bytes.data()); // NOLINT(cppcoreguidelines-pro-type-const-cast)
		auto const data_size = int(bytes.size());
		return io.Fonts->AddFontFromMemoryTTF(data, data_size, size, &config, glyph_ranges) != nullptr;
	}

	void load_default() {
		io.Fonts->AddFontDefault();
		m_merge = true;
	}

	ImGuiIO& io{ImGui::GetIO()};

  private:
	bool m_merge{false};
};
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
	setup_imgui();
	m_frontend.emplace(&m_state);

	while (m_context->next_frame()) {
		m_backend->update();
		draw_frontend();

		m_context->render();
	}

	return EXIT_SUCCESS;
}

void App::create_context() {
	auto window = gvdi::Context::create_window({500.0f, 350.0f}, "riff");
	if (!window) { throw std::runtime_error{"Failed to create Window"}; }

	glfwSetWindowUserPointer(window.get(), this);
	install_callbacks(window.get());

	m_context.emplace(std::move(window));
}

void App::setup_imgui() {
	auto font_loader = ImFontLoader{};
	font_loader.io.Fonts->ClearFonts();
	if (!font_loader.load(rounded_elegance_bytes(), 14.0f)) {
		log.warn("Failed to load RoundedElegance.ttf");
		font_loader.load_default();
	}
	static constexpr auto glyph_ranges_v = std::array<ImWchar, 3>{ICON_MIN_KI, ICON_MAX_KI, 0};
	if (!font_loader.load(kenny_icon_bytes(), 18.0f, glyph_ranges_v.data())) {
		log.error("Failed to load KennyIcons.ttf");
	}
	m_context->rebuild_imgui_fonts();

	static constexpr auto rounding_v = 5.0f;
	auto& style = ImGui::GetStyle();
	style.FrameRounding = style.PopupRounding = style.TabRounding = style.ScrollbarRounding = style.ChildRounding =
		rounding_v;
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

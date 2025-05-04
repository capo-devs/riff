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

	create_player();
	create_context();
	setup_imgui();

	while (m_context->next_frame()) {
		update();
		m_context->render();
	}

	return EXIT_SUCCESS;
}

void App::create_player() {
	m_player.emplace();
	// TODO: saved state
	m_player->set_volume(100);
	m_player->set_balance(0.0f);
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

void App::update() {
	auto const& viewport = *ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport.WorkPos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(viewport.WorkSize);
	static constexpr auto flags_v =
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
	if (ImGui::Begin("main", nullptr, flags_v)) {
		if (m_playing && m_player->at_end()) {
			// TODO: don't wrap if not repeat all
			if (cycle([this] { return m_playlist.cycle_next(); })) { m_player->play(); }
		}
		m_playing = m_player->is_playing();
		update_player();
		ImGui::Separator();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
		update_playlist();
	}
	ImGui::End();
}

void App::update_player() {
	auto const player_action = m_player->update();
	switch (player_action) {
	case Player::Action::None: break;
	case Player::Action::Previous: on_prev(); break;
	case Player::Action::Next: on_next(); break;
	default: break;
	}
}

void App::update_playlist() {
	auto const playlist_action = m_playlist.update();
	switch (playlist_action) {
	case Playlist::Action::None: break;
	case Playlist::Action::Load: {
		auto* track = m_playlist.get_active();
		if (track != nullptr) {
			m_player->load_track(*track);
			if (!m_player->is_playing()) { m_player->play(); }
		}
		break;
	}
	case Playlist::Action::Unload: {
		m_player->unload_track();
		m_playing = false;
		break;
	}
	default: break;
	}
}

template <typename F>
auto App::cycle(F get_track) -> bool {
	while (m_playlist.has_playable_track()) {
		auto* track = get_track();
		if (track == nullptr) { return false; }
		if (m_player->load_track(*track)) { return true; }
		log.error("failed to load file: {}", track->path);
	}
	return false;
}

void App::on_prev() {
	if (m_player->is_playing() && m_player->get_cursor() > 3s) {
		m_player->set_cursor(0s);
		return;
	}
	cycle([this] { return m_playlist.cycle_prev(); });
}

void App::on_next() {
	cycle([this] { return m_playlist.cycle_next(); });
}

void App::on_drop(std::span<char const* const> paths) {
	auto const was_empty = m_playlist.get_tracks().empty();
	for (auto const* path : paths) {
		if (!m_playlist.push(path)) {
			log.info("skipping non-music file: {}", path);
			return;
		}
	}
	if (!was_empty) { return; }

	auto* track = m_playlist.get_active();
	if (track == nullptr) { return; }

	if (!m_player->load_track(*track)) {
		log.error("failed to load file: {}", track->path);
		return;
	}
	m_player->play();
}

void App::install_callbacks(GLFWwindow* window) {
	glfwSetDropCallback(window, [](GLFWwindow* window, int count, char const** paths) {
		self(window).on_drop({paths, std::size_t(count)});
	});
}
} // namespace riff

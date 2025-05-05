#include <IconsKenney.h>
#include <app.hpp>
#include <build_version.hpp>
#include <embedded.hpp>
#include <klib/version_str.hpp>
#include <log.hpp>

namespace riff {
namespace {
[[nodiscard]] auto self(GLFWwindow* window) -> App& { return *static_cast<App*>(glfwGetWindowUserPointer(window)); }

struct ImFontLoader {
	auto load(std::span<std::byte const> bytes, float const size, ImWchar const* glyph_ranges = {}) {
		auto config = ImFontConfig{};
		config.FontDataOwnedByAtlas = false;
		config.MergeMode = std::exchange(m_merge, true);
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

void App::run() {
	create_engine();
	create_player();
	create_context();
	setup_imgui();

	while (m_context->next_frame()) {
		update();
		m_context->render();
	}
}

auto App::play_track(Track& track) -> bool {
	if (!load_track(track)) { return false; }
	if (!m_player->is_playing()) {
		m_player->play();
		m_playing = m_player->is_playing();
	}
	return true;
}

void App::unload_active() {
	m_player->unload_track();
	m_playing = false;
}

void App::skip_prev() {
	auto const is_playing = m_player->is_playing();
	if (is_playing && m_player->get_cursor() > 3s) {
		m_player->set_cursor(0s);
		return;
	}
	if (!cycle([this] { return m_tracklist.cycle_prev(); })) { return; }
	if (is_playing && !m_player->is_playing()) { m_player->play(); }
}

void App::skip_next() {
	auto const is_playing = m_player->is_playing();
	cycle([this] { return m_tracklist.cycle_next(); });
	if (is_playing && !m_player->is_playing()) { m_player->play(); }
}

void App::create_engine() {
	m_engine = capo::create_engine();
	if (!m_engine) { throw std::runtime_error{"Failed to create Audio Engine"}; }
}

void App::create_player() {
	auto source = m_engine->create_source();
	if (!source) { throw std::runtime_error{"Failed to create Audio Source"}; }

	m_player.emplace(std::move(source));
	// TODO: saved state
	m_player->set_volume(100);
	m_player->set_balance(0.0f);
}

void App::create_context() {
	auto const title = std::format("riff {}", klib::to_string(build_version_v));
	auto window = gvdi::Context::create_window({500.0f, 350.0f}, title.c_str());
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
		if (m_playing && m_player->at_end()) { advance(); }
		m_player->update(*this);
		m_playing = m_player->is_playing();

		ImGui::Separator();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
		m_tracklist.update(*this);
	}
	ImGui::End();
}

template <typename F>
auto App::cycle(F get_track) -> bool {
	return cycle([this] { return m_tracklist.has_playable_track(); }, get_track);
}

template <typename Pred, typename F>
auto App::cycle(Pred pred, F get_track) -> bool {
	while (pred()) {
		auto* track = get_track();
		if (track == nullptr) { return false; }
		if (load_track(*track)) { return true; }
	}
	return false;
}

void App::advance() {
	auto const pred = [this] {
		return (m_player->get_repeat() == Player::Repeat::All || m_tracklist.has_next_track()) &&
			   m_tracklist.has_playable_track();
	};
	if (!cycle(pred, [this] { return m_tracklist.cycle_next(); })) { return; }
	m_player->play();
}

auto App::load_track(Track& track) -> bool {
	if (m_player->load_track(track)) { return true; }
	log.error("failed to load track: {}", track.path);
	return false;
}

void App::on_drop(std::span<char const* const> paths) {
	auto const was_empty = m_tracklist.get_tracks().empty();
	for (auto const* path : paths) {
		if (!m_tracklist.push(path)) {
			log.info("skipping non-music file: {}", path);
			return;
		}
	}
	if (!was_empty) { return; }
	advance();
}

void App::install_callbacks(GLFWwindow* window) {
	glfwSetDropCallback(window, [](GLFWwindow* window, int count, char const** paths) {
		self(window).on_drop({paths, std::size_t(count)});
	});
}
} // namespace riff

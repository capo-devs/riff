#include <IconsKenney.h>
#include <app.hpp>
#include <build_version.hpp>
#include <embedded.hpp>
#include <log.hpp>
#include <array>
#include <utility>

namespace riff {
namespace {
[[nodiscard]] auto self(GLFWwindow* window) -> App& { return *static_cast<App*>(glfwGetWindowUserPointer(window)); }

struct ImFontLoader {
	auto load(std::span<std::byte const> bytes, float const size, ImVec2 const offset = {},
			  ImWchar const* glyph_ranges = {}) {
		auto config = ImFontConfig{};
		config.FontDataOwnedByAtlas = false;
		config.MergeMode = std::exchange(m_merge, true);
		config.GlyphOffset = offset;
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

void App::pre_init() {
	m_config.path = m_params.config_path;
	m_config.load_or_create();
	create_engine();
	create_player();

	m_save_playlist.path.set_text("playlist.m3u");
}

auto App::create_window() -> GLFWwindow* {
	auto const title = std::format("riff {}", build_version_str);
	auto* ret = glfwCreateWindow(500, 350, title.c_str(), nullptr, nullptr);
	if (!ret) { throw std::runtime_error{"Failed to create Window"}; }

	glfwSetWindowUserPointer(ret, this);
	install_callbacks(ret);

	return ret;
}

void App::post_init() {
	auto font_loader = ImFontLoader{};
	font_loader.io.Fonts->ClearFonts();
	if (!font_loader.load(rounded_elegance_bytes(), 16.0f)) {
		log.warn("Failed to load RoundedElegance.ttf");
		font_loader.load_default();
	}
	static constexpr auto glyph_ranges_v = std::array<ImWchar, 3>{ICON_MIN_KI, ICON_MAX_KI, 0};
	if (!font_loader.load(kenny_icon_bytes(), 18.0f, {0.0f, 3.0f}, glyph_ranges_v.data())) {
		log.error("Failed to load KennyIcons.ttf");
	}

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
	if (m_save_playlist.update()) { save_playlist(m_save_playlist.path.as_view()); }
	ImGui::End();

	update_config();
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

void App::on_save() { ImGui::OpenPopup(SavePlaylist::label_v.c_str()); }

void App::create_engine() {
	m_engine = capo::create_engine();
	if (!m_engine) { throw std::runtime_error{"Failed to create Audio Engine"}; }
}

void App::create_player() {
	auto source = m_engine->create_source();
	if (!source) { throw std::runtime_error{"Failed to create Audio Source"}; }

	m_player.emplace(std::move(source));
	m_player->set_volume(m_config.get_volume());
	m_player->set_balance(m_config.get_balance());
	m_player->set_repeat(m_config.get_repeat());
}

void App::update_config() {
	m_config.set_volume(m_player->get_volume());
	m_config.set_balance(m_player->get_balance());
	m_config.set_repeat(m_player->get_repeat());
	m_config.update();
}

void App::save_playlist(std::string_view const path) {
	if (m_tracklist.save_playlist(path)) {
		log.info("playlist saved to: {}", path);
		return;
	}

	log.warn("failed to save playlist to: {}", path);
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
		return (m_player->get_repeat() == Repeat::All || m_tracklist.has_next_track()) &&
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
	auto const was_empty = m_tracklist.is_empty();
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

auto App::SavePlaylist::update() -> bool {
	auto ret = false;
	if (imcpp::begin_modal(label_v)) {
		path.update("path");
		ImGui::Separator();
		auto const is_empty = path.as_view().empty();
		if (is_empty) { ImGui::BeginDisabled(); }
		if (ImGui::Button("Save")) {
			ret = true;
			ImGui::CloseCurrentPopup();
		}
		if (is_empty) { ImGui::EndDisabled(); }
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
	return ret;
}
} // namespace riff

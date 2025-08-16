#include <IconsKenney.h>
#include <bin/embedded.hpp>
#include <build_version.hpp>
#include <core/log.hpp>
#include <widgets/app.hpp>
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

[[nodiscard]] constexpr auto to_seek_length(input::Modifier const mods) -> Time {
	using Mod = input::Modifier;
	switch (mods) {
	case Mod::Shift: return 3s;
	case Mod::Alt: return 10s;
	case Mod::Control: return 30s;
	default: return 5s;
	}
}

[[nodiscard]] constexpr auto to_volume_delta(input::Modifier const mods) -> int {
	using Mod = input::Modifier;
	switch (mods) {
	case Mod::Shift: return 2;
	case Mod::Alt: return 10;
	case Mod::Control: return 20;
	default: return 5;
	}
}
} // namespace

void App::pre_init() {
	m_config.path = m_params.config_path;
	m_config.load_or_create();
	create_engine();
	create_player();
	m_tracklist.emplace(&m_events);
	m_controller.emplace(&m_events);

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
		log.warn("failed to load RoundedElegance.ttf");
		font_loader.load_default();
	}
	static constexpr auto glyph_ranges_v = std::array<ImWchar, 3>{ICON_MIN_KI, ICON_MAX_KI, 0};
	if (!font_loader.load(kenny_icon_bytes(), 18.0f, {0.0f, 3.0f}, glyph_ranges_v.data())) {
		log.error("failed to load KennyIcons.ttf");
	}

	static constexpr auto rounding_v = 5.0f;
	auto& style = ImGui::GetStyle();
	style.FrameRounding = style.PopupRounding = style.TabRounding = style.ScrollbarRounding = style.ChildRounding =
		rounding_v;

	bind_events();

	if (auto const autosave_path = m_config.get_autosave_path(); !autosave_path.empty()) {
		m_tracklist->push(autosave_path);
	}
}

void App::update() {
	auto const& viewport = *ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport.WorkPos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(viewport.WorkSize);
	static constexpr auto flags_v =
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
	if (ImGui::Begin("main", nullptr, flags_v)) {
		if (m_was_playing && m_player->at_end()) { advance(); }
		m_player->update();

		ImGui::Separator();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
		m_tracklist->update();
	}
	if (m_save_playlist.update()) { save_playlist(m_save_playlist.path.as_view()); }
	ImGui::End();

	m_was_playing = m_player->is_playing();

	update_config();
}

void App::post_run() {
	if (!m_config.get_autosave() || m_tracklist->is_empty()) { return; }
	auto const autosave_path = m_config.get_autosave_path();
	if (autosave_path.empty()) { return; }
	if (m_tracklist->save_playlist(autosave_path)) {
		log.info("playlist autosaved to: {}", autosave_path);
	} else {
		log.warn("failed to autosave playlist: {}", autosave_path);
	}
}

void App::create_engine() {
	m_engine = capo::create_engine();
	if (!m_engine) { throw std::runtime_error{"Failed to create Audio Engine"}; }
}

void App::create_player() {
	auto source = m_engine->create_source();
	if (!source) { throw std::runtime_error{"Failed to create Audio Source"}; }

	m_player.emplace(std::move(source), &m_events);
	m_player->set_volume(m_config.get_volume());
	m_player->set_balance(m_config.get_balance());
	m_player->set_repeat(m_config.get_repeat());
}

void App::bind_events() {
	m_events.save.bind([] { ImGui::OpenPopup(SavePlaylist::label_v.c_str()); });
	m_events.unload_active.bind([this] { unload_active(); });
	m_events.play_track.bind([this](Track* track) {
		if (!play_track(*track)) { m_tracklist->reset_active(); }
	});
	m_events.skip.bind([this](Polarity const polarity) {
		switch (polarity) {
		case Polarity::Negative: skip_prev(); break;
		case Polarity::Positive: skip_next(); break;
		default: break;
		}
	});
	m_events.toggle_playback.bind([this] { toggle_playback(); });
	m_events.seek.bind([this](Polarity const polarity, input::Modifier const mods) { seek(polarity, mods); });
	m_events.toggle_repeat.bind([this] { toggle_repeat(); });
	m_events.adjust_volume.bind(
		[this](Polarity const polarity, input::Modifier const mods) { adjust_volume(polarity, mods); });
}

void App::update_config() {
	m_config.set_volume(m_player->get_volume());
	m_config.set_balance(m_player->get_balance());
	m_config.set_repeat(m_player->get_repeat());
	m_config.update();
}

auto App::play_track(Track& track) -> bool {
	if (!load_track(track)) { return false; }
	if (!m_player->is_playing()) {
		m_player->play();
		m_was_playing = m_player->is_playing();
	}
	return true;
}

void App::unload_active() {
	m_player->unload_track();
	m_was_playing = false;
}

void App::skip_prev() {
	auto const is_playing = m_player->is_playing();
	if (is_playing && m_player->get_cursor() > 3s) {
		m_player->set_cursor(0s);
		return;
	}
	if (!cycle([this] { return m_tracklist->cycle_prev(); })) { return; }
	if (is_playing && !m_player->is_playing()) { m_player->play(); }
}

void App::skip_next() {
	auto const is_playing = m_player->is_playing();
	cycle([this] { return m_tracklist->cycle_next(); });
	if (is_playing && !m_player->is_playing()) { m_player->play(); }
}

void App::toggle_playback() {
	if (m_player->is_playing()) {
		m_player->pause();
	} else {
		if (!m_player->is_track_loaded() && m_tracklist->has_next_track()) { skip_next(); }
		m_player->play();
	}
	m_was_playing = m_player->is_playing();
}

void App::seek(Polarity const polarity, input::Modifier const mods) {
	auto const sign = to_f32(polarity);
	auto const length = to_seek_length(mods);
	m_player->set_cursor(m_player->get_cursor() + sign * length);
}

void App::toggle_repeat() {
	auto const current = std::to_underlying(m_player->get_repeat());
	auto const next = Repeat((current + 1) % std::to_underlying(Repeat::COUNT_));
	m_player->set_repeat(next);
}

void App::adjust_volume(Polarity const polarity, input::Modifier const mods) {
	auto const sign = to_int(polarity);
	auto const delta = sign * to_volume_delta(mods);
	m_player->set_volume(m_player->get_volume() + delta);
}

void App::save_playlist(std::string_view const path) {
	if (m_tracklist->save_playlist(path)) {
		log.info("playlist saved to: {}", path);
		return;
	}

	log.warn("failed to save playlist to: {}", path);
}

template <typename F>
auto App::cycle(F get_track) -> bool {
	return cycle([this] { return m_tracklist->has_playable_track(); }, get_track);
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
		return (m_player->get_repeat() == Repeat::All || m_tracklist->has_next_track()) &&
			   m_tracklist->has_playable_track();
	};
	if (!cycle(pred, [this] { return m_tracklist->cycle_next(); })) { return; }
	m_player->play();
}

auto App::load_track(Track& track) -> bool {
	if (m_player->load_track(track)) { return true; }
	log.error("failed to load track: {}", track.path);
	return false;
}

void App::on_drop(std::span<char const* const> paths) {
	auto const was_empty = m_tracklist->is_empty();
	for (auto const* path : paths) {
		if (!m_tracklist->push(path)) {
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
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int /*scancode*/, int action, int mods) {
		self(window).m_controller->on_key(key, action, mods);
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

#include <IconsKenney.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <capo/format.hpp>
#include <klib/enum_array.hpp>
#include <player.hpp>
#include <util.hpp>
#include <cassert>
#include <utility>

namespace riff {
namespace {
auto const duration_0_str = capo::format_duration(0s);

constexpr auto repeat_icon_v = klib::EnumArray<Repeat, klib::CString>{
	ICON_KI_MINUS,
	ICON_KI_MOVE_RL_ALT,
	ICON_KI_STICK_MOVE_RL_ALT,
};
} // namespace

Player::Player(std::unique_ptr<capo::ISource> source) : m_source(std::move(source)) {
	assert(m_source);
	m_cursor_str = duration_0_str;
	m_duration_str = duration_0_str.c_str();
}

void Player::set_repeat(Repeat const repeat) {
	m_repeat = repeat;
	m_source->set_looping(m_repeat == Repeat::One);
}

auto Player::load_track(Track& track) -> bool {
	auto const was_playing = is_playing();
	if (!m_source->open_file_stream(track.path.c_str())) {
		track.status = Track::Status::Error;
		return false;
	}

	track.status = Track::Status::Ok;
	track.duration = m_source->get_duration();
	track.duration_label.clear();
	capo::format_duration_to(track.duration_label, track.duration);

	m_title = track.name.c_str();
	m_duration_str = track.duration_label.c_str();
	m_seeking = false;

	if (was_playing) { play(); }
	return true;
}

void Player::unload_track() {
	m_source->unbind();

	m_title = blank_title_v.data();
	m_duration_str = duration_0_str.c_str();
	m_seeking = false;
}

void Player::update(IMediator& mediator) {
	if (!m_seeking) { m_cursor = std::max(m_source->get_cursor().count(), 0.0f); }
	m_cursor_str.clear();
	capo::format_duration_to(m_cursor_str, Time{m_cursor});

	ImGui::TextUnformatted(m_title.c_str());
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
	buttons(mediator);
	sliders();
	seekbar();
}

void Player::buttons(IMediator& mediator) {
	ImGui::SetNextItemWidth(50.0f);
	if (!m_source->is_bound()) { ImGui::BeginDisabled(); }
	if (ImGui::ButtonEx(m_source->is_playing() ? ICON_KI_PAUSE : ICON_KI_CARET_RIGHT, {50.0f, 50.0f})) {
		if (m_source->is_playing()) {
			m_source->stop();
		} else {
			m_source->play();
		}
	}

	enum class Action : std::int8_t { None, Previous, Next };
	auto action = Action::None;

	ImGui::SameLine();
	if (ImGui::ButtonEx(ICON_KI_STEP_BACKWARD, {30.0f, 30.0f})) { action = Action::Previous; }
	ImGui::SameLine();
	if (ImGui::ButtonEx(ICON_KI_STEP_FORWARD, {30.0f, 30.0f})) { action = Action::Next; }
	if (!m_source->is_bound()) { ImGui::EndDisabled(); }

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetStyle().ItemSpacing.x);
	auto const repeat_icon = repeat_icon_v[m_repeat];
	if (ImGui::ButtonEx(repeat_icon.c_str(), {30.0f, 30.0f})) {
		set_repeat(Repeat((int(m_repeat) + 1) % int(Repeat::COUNT_)));
	}

	switch (action) {
	case Action::None: break;
	case Action::Previous: mediator.skip_prev(); break;
	case Action::Next: mediator.skip_next(); break;
	}
}

void Player::sliders() {
	static constexpr auto volume_width_v = 100.0f;
	static constexpr auto balance_width_v = 100.0f;
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 50.0f);

	auto const balance_icon_size = ImGui::CalcTextSize(ICON_KI_SORT_HORIZONTAL);
	util::align_right(balance_icon_size.x, balance_width_v);
	ImGui::TextUnformatted(ICON_KI_SORT_HORIZONTAL);
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - (0.2f * balance_icon_size.y));
	ImGui::SetNextItemWidth(balance_width_v);
	auto balance = m_source->get_pan();
	if (ImGui::SliderFloat("##balance", &balance, -1.0f, 1.0f, "%.1f")) { m_source->set_pan(balance); }
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (0.2f * balance_icon_size.y));

	auto const volume_icon_size = ImGui::CalcTextSize(ICON_KI_SOUND_ON);
	util::align_right(volume_icon_size.x, volume_width_v);
	ImGui::TextUnformatted(ICON_KI_SOUND_ON);
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - (0.2f * volume_icon_size.y));
	ImGui::SetNextItemWidth(volume_width_v);
	auto volume = int(m_source->get_gain() * 100.0f);
	if (ImGui::SliderInt("##volume", &volume, 0, 100, "%d", ImGuiSliderFlags_ClampZeroRange)) {
		m_source->set_gain(float(volume) * 0.01f);
	}
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (0.2f * volume_icon_size.y));
}

void Player::seekbar() {
	ImGui::NewLine();
	ImGui::TextUnformatted(duration_0_str.c_str());
	auto const duration_width = ImGui::CalcTextSize(m_duration_str.c_str()).x;
	ImGui::SameLine();
	util::align_right(duration_width);
	ImGui::TextUnformatted(m_duration_str.c_str());

	auto fduration = std::max(m_source->get_duration().count(), 0.0f);
	if (fduration == 0.0f) { ImGui::BeginDisabled(); }
	ImGui::SetNextItemWidth(-1.0f);
	static constexpr auto flags_v = ImGuiSliderFlags_NoInput;
	ImGui::SliderFloat("##cursor", &m_cursor, 0.0f, fduration, m_cursor_str.c_str(), flags_v);
	if (ImGui::IsItemClicked()) { m_seeking = true; }
	auto const was_seeking = m_seeking;
	if (m_seeking && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) { m_seeking = false; }
	if (was_seeking && !m_seeking) { m_source->set_cursor(Time{m_cursor}); }
	if (fduration == 0.0f) { ImGui::EndDisabled(); }
}
} // namespace riff

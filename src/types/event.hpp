#pragma once
#include <core/signal.hpp>
#include <input/modifier.hpp>
#include <types/polarity.hpp>
#include <types/track.hpp>

namespace riff {
namespace event {
struct Save : Signal<> {};
struct UnloadActive : Signal<> {};
struct PlayTrack : Signal<Track*> {};
struct Skip : Signal<Polarity> {};
struct TogglePlayback : Signal<> {};
struct Seek : Signal<Polarity, input::Modifier> {};
struct ToggleRepeat : Signal<> {};
struct AdjustVolume : Signal<Polarity, input::Modifier> {};
} // namespace event

struct Events {
	event::Skip skip{};
	event::Save save{};
	event::UnloadActive unload_active{};
	event::PlayTrack play_track{};
	event::TogglePlayback toggle_playback{};
	event::Seek seek{};
	event::ToggleRepeat toggle_repeat{};
	event::AdjustVolume adjust_volume{};
};
} // namespace riff

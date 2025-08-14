#pragma once
#include <core/signal.hpp>
#include <types/track.hpp>

namespace riff {
namespace event {
struct Save : Signal<> {};
struct UnloadActive : Signal<> {};
struct PlayTrack : Signal<Track*> {};
struct SkipPrev : Signal<> {};
struct SkipNext : Signal<> {};
} // namespace event

struct Events {
	event::SkipPrev skip_prev{};
	event::SkipNext skip_next{};
	event::Save save{};
	event::UnloadActive unload_active{};
	event::PlayTrack play_track{};
};
} // namespace riff

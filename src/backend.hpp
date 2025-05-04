#pragma once
#include <capo/engine.hpp>
#include <state.hpp>
#include <span>

namespace riff {
class Backend {
  public:
	explicit Backend(State* state);

	void on_drop(std::span<char const* const> paths);

	void update();

  private:
	auto check_track(Track& track) -> bool;

	void play_previous();
	void play_next(bool pre_increment);
	auto play_track(Track& track) -> bool;

	template <typename Pred, typename Post>
	void try_play(Pred pred, Post post);

	State* m_state{};

	std::unique_ptr<capo::IEngine> m_engine{};
	std::unique_ptr<capo::ISource> m_source{};
	std::unique_ptr<capo::ISource> m_checker{};
};
} // namespace riff

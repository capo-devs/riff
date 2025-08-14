#pragma once
#include <input/mapping.hpp>
#include <types/event.hpp>
#include <gsl/pointers>

namespace riff::input {
class Controller {
  public:
	explicit Controller(gsl::not_null<Events*> events);

	void on_key(int key, int action, int mods);

  private:
	void add_bindings();

	gsl::not_null<Events*> m_events;

	Mapping m_key_mapping{};
};
} // namespace riff::input

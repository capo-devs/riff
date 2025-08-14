#pragma once
#include <functional>
#include <vector>

namespace riff {
template <typename... Args>
class Signal {
  public:
	using Callback = std::move_only_function<void(Args...)>;

	void bind(Callback callback) { m_callbacks.push_back(std::move(callback)); }

	void clear_bindings() { m_callbacks.clear(); }

	void dispatch(Args... args) {
		for (auto& callback : m_callbacks) { callback(args...); }
	}

	void operator()(Args... args) { dispatch(args...); }

  protected:
	std::vector<Callback> m_callbacks{};
};
} // namespace riff

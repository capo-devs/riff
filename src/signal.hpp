#pragma once
#include <functional>
#include <vector>

namespace riff {
template <typename... Args>
class Signal {
  public:
	using Callback = std::move_only_function<void(Args const&...)>;

	void connect(Callback callback) {
		if (!callback) { return; }
		m_callbacks.push_back(std::move(callback));
	}

	void dispatch(Args const&... args) {
		for (auto& callback : m_callbacks) { callback(args...); }
	}

  private:
	std::vector<Callback> m_callbacks{};
};
} // namespace riff

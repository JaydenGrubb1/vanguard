#pragma once

#include <span>
#include <string_view>

namespace vg {

class App {
  public:
	App(std::span<const std::string_view> args);
	~App();

	void run();
	void quit();

  private:
	bool m_running = false;
};

} // namespace vg

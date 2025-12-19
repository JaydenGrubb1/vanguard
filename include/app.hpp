#pragma once

#include <SDL3/SDL.h>

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

	SDL_Window* m_window = nullptr;
};

} // namespace vg

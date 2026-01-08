#pragma once

#include <SDL3/SDL.h>

#include <span>
#include <string_view>

#include "gfx/device.hpp"
#include "gfx/renderer.hpp"

namespace vg {

class App {
  public:
	explicit App(std::span<const std::string_view> args);
	~App();

	void run();
	void quit();

  private:
	bool m_running = false;

	SDL_Window* m_window = nullptr;
	std::unique_ptr<gfx::IDevice> m_device;
	std::unique_ptr<gfx::Renderer> m_renderer;
};

} // namespace vg

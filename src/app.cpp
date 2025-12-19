#include <print>
#include <ranges>
#include <stdexcept>

#include "app.hpp"

namespace vg {

App::App(std::span<const std::string_view> args) {
	for (auto [idx, arg] : std::views::enumerate(args)) {
		std::println("arg[{}] = {}", idx, arg);
	}

	if (!SDL_Init(SDL_INIT_VIDEO))
		throw std::runtime_error("Failed to initialize SDL");

	m_window = SDL_CreateWindow("Vanguard", 1600, 900, SDL_WINDOW_RESIZABLE);
	if (m_window == nullptr)
		throw std::runtime_error("Failed to create window");
}

App::~App() {
	if (m_window != nullptr)
		SDL_DestroyWindow(m_window);

	SDL_Quit();
}

void App::run() {
	m_running = true;

	while (m_running) {
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_EVENT_QUIT:
					quit();
					break;
				default:
					break;
			}
		}

		// render frame
	}
}

void App::quit() {
	m_running = false;
}

} // namespace vg

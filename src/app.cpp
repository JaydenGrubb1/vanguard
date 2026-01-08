#include <nvrhi/utils.h>

#include <filesystem>
#include <print>
#include <ranges>
#include <stdexcept>

#include "app.hpp"

namespace vg {

class ClearColorPass : public gfx::IRenderPass {
  public:
	explicit ClearColorPass(const nvrhi::Color color = nvrhi::Color(0.f)) : m_color(color) {}

	void render(nvrhi::IFramebuffer* framebuffer, nvrhi::ICommandList* command_list) override {
		nvrhi::utils::ClearColorAttachment(command_list, framebuffer, 0, m_color);
		nvrhi::utils::ClearDepthStencilAttachment(command_list, framebuffer, 1.f, 0);
	}

  private:
	nvrhi::Color m_color;
};

App::App(std::span<const std::string_view> args) {
	for (auto [idx, arg] : std::views::enumerate(args)) {
		std::println("arg[{}] = {}", idx, arg);
	}

	if (!SDL_Init(SDL_INIT_VIDEO))
		throw std::runtime_error("Failed to initialize SDL");

	m_window = SDL_CreateWindow("Vanguard", 1600, 900, SDL_WINDOW_RESIZABLE);
	if (m_window == nullptr)
		throw std::runtime_error("Failed to create window");

	std::println("current_path: {}", std::filesystem::current_path().string());

	m_device = gfx::IDevice::create();
	m_device->create_swapchain(m_window);
	m_device->resize_swapchain();

	m_renderer = std::make_unique<gfx::Renderer>(*m_device);
	m_renderer->add_render_pass<ClearColorPass>(nvrhi::Color(1.f, 0.f, 0.f, 1.f));
	m_renderer->init();
}

App::~App() {
	if (m_window != nullptr)
		SDL_DestroyWindow(m_window);

	SDL_Quit();
}

void App::run() {
	m_running = true;

	const auto frequency = static_cast<float>(SDL_GetPerformanceFrequency());
	u64 now = SDL_GetPerformanceCounter();
	u64 last = 0;

	while (m_running) {
		last = now;
		now = SDL_GetPerformanceCounter();
		const auto elapsed = static_cast<float>(now - last);
		const float delta = elapsed / frequency;

		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_EVENT_QUIT:
					quit();
					break;
				case SDL_EVENT_WINDOW_RESIZED:
					m_device->resize_swapchain();
					break;
				default:
					break;
			}
		}

		std::ignore = delta; // TODO: Update scene
		m_renderer->render();
	}
}

void App::quit() {
	m_running = false;
}

} // namespace vg

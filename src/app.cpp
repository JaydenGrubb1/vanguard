#include <nvrhi/utils.h>

#include <filesystem>
#include <print>
#include <ranges>
#include <stdexcept>

#include "app.hpp"

namespace vg {

class GeometryPass : public gfx::IRenderPass {
  public:
	void init(nvrhi::IDevice* device, const u32 width, const u32 height) override {
		auto desc =
			nvrhi::TextureDesc()
				.setDimension(nvrhi::TextureDimension::Texture2D)
				.setWidth(width)
				.setHeight(height)
				.setIsRenderTarget(true)
				.setKeepInitialState(true);

		m_color = device->createTexture(
			desc.setDebugName("clear_pass_color")
				.setFormat(nvrhi::Format::RGBA16_FLOAT)
				.setInitialState(nvrhi::ResourceStates::RenderTarget)
		);
		m_depth = device->createTexture(
			desc.setDebugName("clear_color_depth")
				.setFormat(nvrhi::Format::D32)
				.setInitialState(nvrhi::ResourceStates::DepthWrite)
		);

		m_framebuffer =
			device->createFramebuffer(nvrhi::FramebufferDesc().addColorAttachment(m_color).setDepthAttachment(m_depth));
	}

	void render(nvrhi::ICommandList* command_list, gfx::RenderPassContext& ctx) override {
		nvrhi::utils::ClearColorAttachment(command_list, m_framebuffer, 0, nvrhi::Color(1.0f, 0.0f, 0.0f, 1.0f));
		nvrhi::utils::ClearDepthStencilAttachment(command_list, m_framebuffer, 1.0f, 0);

		ctx.set_resource("color", m_color);
		ctx.set_resource("depth", m_depth);
	}

  private:
	nvrhi::TextureHandle m_color;
	nvrhi::TextureHandle m_depth;
	nvrhi::FramebufferHandle m_framebuffer;
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
	SDL_SetWindowMinimumSize(m_window, 255, 255);

	std::println("current_path: {}", std::filesystem::current_path().string());

	m_device = gfx::IDevice::create();
	m_device->create_swapchain(m_window);
	m_device->resize_swapchain();

	m_renderer = std::make_unique<gfx::Renderer>(*m_device);
	m_renderer->add_render_pass<GeometryPass>(gfx::RenderPassInfo().set_name("clear_color"));
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
					m_renderer->init();
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

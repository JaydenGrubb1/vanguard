#include <nvrhi/utils.h>

#include <ranges>

#include "gfx/renderer.hpp"
#include "gfx/shader.hpp"

namespace vg::gfx {

void RenderPassContext::set_resource(const std::string& name, nvrhi::ResourceHandle resource) {
	m_resources[name] = std::move(resource);
}

nvrhi::ResourceHandle RenderPassContext::get_resource(const std::string& name) const {
	const auto iter = m_resources.find(name);
	if (iter == m_resources.end())
		throw std::runtime_error("RenderPassContext::get: unknown texture name");
	return iter->second;
}

class DefaultOutputPass : public IOutputPass {
  public:
	void init(nvrhi::IDevice* device, const u32 width, const u32 height) override {
		m_viewport = nvrhi::Viewport(static_cast<float>(width), static_cast<float>(height));

		const auto vert_shader = Shader::from_binary("shaders/blit.vs.dxil", nvrhi::ShaderType::Vertex, device);
		const auto pixel_shader = Shader::from_binary("shaders/blit.ps.dxil", nvrhi::ShaderType::Pixel, device);

		m_binding_layout = device->createBindingLayout(
			nvrhi::BindingLayoutDesc()
				.setVisibility(nvrhi::ShaderType::All)
				.addItem(nvrhi::BindingLayoutItem::Texture_SRV(0))
				.addItem(nvrhi::BindingLayoutItem::Sampler(0))
		);
		m_binding_set = nullptr;

		m_pipeline = device->createGraphicsPipeline(
			nvrhi::GraphicsPipelineDesc()
				.setVertexShader(vert_shader)
				.setPixelShader(pixel_shader)
				.addBindingLayout(m_binding_layout)
				.setRenderState(nvrhi::RenderState().setDepthStencilState(nvrhi::DepthStencilState().disableDepthTest())),
			nvrhi::FramebufferInfo().addColorFormat(nvrhi::Format::SRGBA8_UNORM)
		);
	}

	void render(nvrhi::ICommandList* command_list, RenderPassContext& ctx) override {
		const auto input = ctx.get_resource<nvrhi::TextureHandle>("color");

		command_list->setTextureState(input, nvrhi::AllSubresources, nvrhi::ResourceStates::ShaderResource);

		if (!m_binding_set) {
			const auto device = command_list->getDevice();
			const auto sampler = device->createSampler(nvrhi::SamplerDesc());

			m_binding_set = device->createBindingSet(
				nvrhi::BindingSetDesc()
					.addItem(nvrhi::BindingSetItem::Texture_SRV(0, input))
					.addItem(nvrhi::BindingSetItem::Sampler(0, sampler)),
				m_binding_layout
			);
		}

		nvrhi::utils::ClearColorAttachment(command_list, m_framebuffer, 0, nvrhi::Color(0.0f));

		command_list->setGraphicsState(
			nvrhi::GraphicsState()
				.setPipeline(m_pipeline)
				.setFramebuffer(m_framebuffer)
				.setViewport(nvrhi::ViewportState().addViewportAndScissorRect(m_viewport))
				.addBindingSet(m_binding_set)
		);

		command_list->draw(nvrhi::DrawArguments().setVertexCount(6));
	}

	void set_framebuffer(nvrhi::IFramebuffer* framebuffer) override {
		m_framebuffer = framebuffer;
	}

  private:
	nvrhi::IFramebuffer* m_framebuffer = nullptr;
	nvrhi::BindingLayoutHandle m_binding_layout;
	nvrhi::BindingSetHandle m_binding_set;
	nvrhi::GraphicsPipelineHandle m_pipeline;
	nvrhi::Viewport m_viewport;
};

Renderer::Renderer(IDevice& device) : m_device(device) {
	m_command_list = m_device.get_device()->createCommandList();
	m_output_pass = std::make_unique<DefaultOutputPass>();
}

void Renderer::init() const {
	u32 width, height;
	m_device.get_swapchain_size(width, height);

	for (const auto& pass : std::views::values(m_render_passes)) {
		pass->init(m_device.get_device(), width, height);
	}
	m_output_pass->init(m_device.get_device(), width, height);
}

void Renderer::render() {
	const auto framebuffer = m_device.begin_frame();
	m_command_list->open();

	for (const auto& [info, pass] : m_render_passes) {
#ifndef NDEBUG
		m_command_list->beginMarker(info.name.c_str());
#endif
		pass->render(m_command_list, m_render_pass_context);
#ifndef NDEBUG
		m_command_list->endMarker();
#endif
	}

#ifndef NDEBUG
	m_command_list->beginMarker("output_pass");
#endif
	m_output_pass->set_framebuffer(framebuffer);
	m_output_pass->render(m_command_list, m_render_pass_context);
#ifndef NDEBUG
	m_command_list->endMarker();
#endif

	m_command_list->close();
	m_device.get_device()->executeCommandList(m_command_list);

	m_device.end_frame();
}

} // namespace vg::gfx

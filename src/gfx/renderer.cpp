#include "gfx/renderer.hpp"

namespace vg::gfx {

Renderer::Renderer(IDevice& device) : m_device(device) {
	m_command_list = m_device.get_device()->createCommandList();
}

void Renderer::init() const {
	for (const auto& pass : m_render_passes) {
		pass->init(m_device.get_device());
	}
}

void Renderer::render() const {
	const auto framebuffer = m_device.begin_frame();

	for (const auto& pass : m_render_passes) {
		m_command_list->open();

		pass->render(framebuffer, m_command_list);

		m_command_list->close();
		m_device.get_device()->executeCommandList(m_command_list);
	}

	m_device.end_frame();
}

} // namespace vg::gfx

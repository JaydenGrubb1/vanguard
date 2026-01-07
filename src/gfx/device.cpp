#include <print>

#include "backends/dx12/device.hpp"
#include "gfx/device.hpp"

namespace vg::gfx {

std::unique_ptr<IDevice> IDevice::create() {
	return std::make_unique<DX12Device>();
}

void IDevice::message(const nvrhi::MessageSeverity severity, const char* text) {
	std::string_view level;

	switch (severity) {
		case nvrhi::MessageSeverity::Info:
			level = "Info";
			break;
		case nvrhi::MessageSeverity::Warning:
			level = "Warning";
			break;
		case nvrhi::MessageSeverity::Error:
			level = "Error";
			break;
		default:
			level = "Unknown";
			break;
	}

	std::println("[nvrhi][{}]: {}", level, text);
}

nvrhi::FramebufferHandle IDevice::begin_frame() {
	acquire_frame();
	return m_framebuffers[get_current_index()];
}

void IDevice::end_frame() {
	present_frame();
}

void IDevice::create_depth_buffer(const u32 width, const u32 height) {
	m_depth_texture.Reset();

	nvrhi::TextureDesc desc = {};
	desc.setDebugName("depth_buffer");
	desc.setWidth(width);
	desc.setHeight(height);
	desc.setFormat(nvrhi::Format::D32);
	desc.setDimension(nvrhi::TextureDimension::Texture2D);
	desc.setSampleCount(1);
	desc.setSampleQuality(0);
	desc.setIsTypeless(true);
	desc.setIsRenderTarget(true);
	desc.enableAutomaticStateTracking(nvrhi::ResourceStates::DepthWrite);
	desc.setUseClearValue(true);
	desc.setClearValue(nvrhi::Color(1, 0, 0, 0));

	m_depth_texture = get_device()->createTexture(desc);
}

void IDevice::create_framebuffers() {
	const auto tex_desc = get_buffer(0)->getDesc();
	create_depth_buffer(tex_desc.width, tex_desc.height);

	// TODO: signal render passes

	const u32 count = get_buffer_count();
	m_framebuffers.resize(count);

	for (u32 i = 0; i < count; i++) {
		nvrhi::FramebufferDesc desc = {};
		desc.addColorAttachment(get_buffer(i));
		desc.setDepthAttachment(m_depth_texture);
		m_framebuffers[i] = get_device()->createFramebuffer(desc);
	}
}

void IDevice::destroy_framebuffers() {
	m_framebuffers.clear();
	// TODO: signal render passes
}

} // namespace vg::gfx

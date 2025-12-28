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

void IDevice::create_framebuffers() {
	// TODO: signal render passes

	const u32 count = get_buffer_count();
	m_framebuffers.resize(count);

	for (u32 i = 0; i < count; i++) {
		nvrhi::FramebufferDesc desc = {};
		desc.addColorAttachment(get_buffer(i));
		m_framebuffers[i] = get_device()->createFramebuffer(desc);
	}
}

void IDevice::destroy_framebuffers() {
	m_framebuffers.clear();
	// TODO: signal render passes
}

} // namespace vg::gfx

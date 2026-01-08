#include "gfx/render_pass.hpp"

namespace vg::gfx {

void IRenderPass::init(nvrhi::IDevice* device) {
	// NOOP
	std::ignore = device;
}

void IRenderPass::render(nvrhi::IFramebuffer* framebuffer, nvrhi::ICommandList* command_list) {
	// NOOP
	std::ignore = framebuffer;
	std::ignore = command_list;
}

} // namespace vg::gfx

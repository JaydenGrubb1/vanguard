#include "gfx/render_pass.hpp"
#include "gfx/renderer.hpp"

namespace vg::gfx {

void IRenderPass::init(nvrhi::IDevice*, u32, u32) {
	// NOOP
}

void IRenderPass::render(nvrhi::ICommandList*, RenderPassContext&) {
	// NOOP
}

} // namespace vg::gfx

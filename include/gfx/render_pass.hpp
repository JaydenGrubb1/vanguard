#pragma once

#include <nvrhi/nvrhi.h>

namespace vg::gfx {

class IRenderPass {
  public:
	virtual ~IRenderPass() = default;

	virtual void init(nvrhi::IDevice* device);
	virtual void render(nvrhi::IFramebuffer* framebuffer, nvrhi::ICommandList* command_list);
};

} // namespace vg::gfx

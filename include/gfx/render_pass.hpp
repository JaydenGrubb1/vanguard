#pragma once

#include <nvrhi/nvrhi.h>

#include <string>

#include "types.hpp"

namespace vg::gfx {

struct RenderPassInfo {
	std::string name;

	constexpr RenderPassInfo& set_name(const std::string& value) {
		name = value;
		return *this;
	}
};

class RenderPassContext;

class IRenderPass {
  public:
	virtual ~IRenderPass() = default;

	virtual void init(nvrhi::IDevice* device, u32 width, u32 height);
	virtual void render(nvrhi::ICommandList* command_list, RenderPassContext& ctx);
};

} // namespace vg::gfx

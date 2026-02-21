#pragma once

#include <vulkan/vulkan.h>

#include "gfx/device.hpp"

namespace vg::gfx {

class VulkanDevice final : public IDevice {
  public:
	VulkanDevice();
	~VulkanDevice() override;

	void create_swapchain(SDL_Window* window) override;
	void destroy_swapchain() override;
	void resize_swapchain() override;

	void create_render_targets() override;
	void destroy_render_targets() override;

	void acquire_frame() override;
	void present_frame() override;

	u32 get_current_index() override;
	u32 get_buffer_count() override;
	nvrhi::TextureHandle get_buffer(u32 index) override;
	nvrhi::DeviceHandle get_device() override;
};

} // namespace vg::gfx

#pragma once

#ifndef VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
	#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#endif
#include <vulkan/vulkan.hpp>

#include "gfx/device.hpp"

#if VK_HEADER_VERSION >= 301
using VkDynamicLoader = vk::detail::DynamicLoader;
#else
using VkDynamicLoader = vk::DynamicLoader;
#endif

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

  private:
	std::unique_ptr<VkDynamicLoader> m_loader;

	vk::Instance m_instance;
	vk::DebugUtilsMessengerEXT m_debug;
};

} // namespace vg::gfx

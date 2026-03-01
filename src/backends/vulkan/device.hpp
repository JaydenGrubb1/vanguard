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
	nvrhi::DeviceHandle m_handle;

	std::unique_ptr<VkDynamicLoader> m_loader;

	std::vector<const char*> m_instance_layers;
	std::vector<const char*> m_instance_extensions;
	std::vector<const char*> m_device_extensions;

	vk::Instance m_instance;
	vk::DebugUtilsMessengerEXT m_debug;
	vk::PhysicalDevice m_physical_device;
	vk::Device m_device;

	int m_graphics_queue_index = -1;
	int m_compute_queue_index = -1;
	int m_transfer_queue_index = -1;
	int m_present_queue_index = -1;

	vk::Queue m_graphics_queue;
	vk::Queue m_compute_queue;
	vk::Queue m_transfer_queue;
	vk::Queue m_present_queue;
};

} // namespace vg::gfx

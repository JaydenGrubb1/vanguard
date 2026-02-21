#include <nvrhi/validation.h>
#include <nvrhi/vulkan.h>

#include <stdexcept>

#include "backends/vulkan/device.hpp"

namespace vg::gfx {

VulkanDevice::VulkanDevice() {
	throw std::runtime_error("VulkanDevice::VulkanDevice() not implemented");
}

VulkanDevice::~VulkanDevice() {
	// TODO
}

void VulkanDevice::create_swapchain(SDL_Window*) {
	throw std::runtime_error("VulkanDevice::create_swapchain() not implemented");
}

void VulkanDevice::destroy_swapchain() {
	throw std::runtime_error("VulkanDevice::destroy_swapchain() not implemented");
}

void VulkanDevice::resize_swapchain() {
	throw std::runtime_error("VulkanDevice::resize_swapchain() not implemented");
}

void VulkanDevice::create_render_targets() {
	throw std::runtime_error("VulkanDevice::create_render_targets() not implemented");
}

void VulkanDevice::destroy_render_targets() {
	throw std::runtime_error("VulkanDevice::destroy_render_targets() not implemented");
}

void VulkanDevice::acquire_frame() {
	throw std::runtime_error("VulkanDevice::acquire_frame() not implemented");
}

void VulkanDevice::present_frame() {
	throw std::runtime_error("VulkanDevice::present_frame() not implemented");
}

u32 VulkanDevice::get_current_index() {
	throw std::runtime_error("VulkanDevice::get_current_index() not implemented");
}

u32 VulkanDevice::get_buffer_count() {
	throw std::runtime_error("VulkanDevice::get_buffer_count() not implemented");
}

nvrhi::TextureHandle VulkanDevice::get_buffer(u32) {
	throw std::runtime_error("VulkanDevice::get_buffer() not implemented");
}

nvrhi::DeviceHandle VulkanDevice::get_device() {
	throw std::runtime_error("VulkanDevice::get_device() not implemented");
}

} // namespace vg::gfx

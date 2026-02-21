#include <nvrhi/validation.h>
#include <nvrhi/vulkan.h>

#include <print>
#include <stdexcept>

#include "backends/vulkan/device.hpp"

// needs to be defined in exactly one cpp file
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

static constexpr auto MIN_VULKAN_VERSION = VK_API_VERSION_1_2;

template<>
struct std::formatter<vk::DebugUtilsMessageSeverityFlagBitsEXT> : std::formatter<std::string_view> {
	template<typename FormatCtx>
	auto format(const vk::DebugUtilsMessageSeverityFlagBitsEXT severity, FormatCtx& ctx) const {
		switch (severity) {
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
				return std::formatter<std::string_view>::format("VERBOSE", ctx);
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
				return std::formatter<std::string_view>::format("INFO", ctx);
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
				return std::formatter<std::string_view>::format("WARNING", ctx);
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
				return std::formatter<std::string_view>::format("ERROR", ctx);
			default:
				return std::formatter<std::string_view>::format("?", ctx);
		}
	}
};

namespace vg::gfx {

#ifndef NDEBUG
static VKAPI_ATTR vk::Bool32 VKAPI_CALL debug_callback(
	vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
	vk::DebugUtilsMessageTypeFlagsEXT,
	const vk::DebugUtilsMessengerCallbackDataEXT* data,
	void*
) {
	std::println("[vk][{}]: {}", severity, data->pMessage);
	return VK_FALSE;
}
#endif

VulkanDevice::VulkanDevice() {
	m_loader = std::make_unique<VkDynamicLoader>();
	VULKAN_HPP_DEFAULT_DISPATCHER.init(m_loader->getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

	auto app =
		vk::ApplicationInfo()
			.setPApplicationName("Vanguard")
			.setApplicationVersion(VK_MAKE_API_VERSION(0, 1, 0, 0))
			.setPEngineName("VG")
			.setEngineVersion(VK_MAKE_API_VERSION(0, 1, 0, 0));

	std::ignore = vk::enumerateInstanceVersion(&app.apiVersion);

	std::println(
		"vk::version: {}.{}.{}-{}",
		VK_API_VERSION_MAJOR(app.apiVersion),
		VK_API_VERSION_MINOR(app.apiVersion),
		VK_API_VERSION_PATCH(app.apiVersion),
		VK_API_VERSION_VARIANT(app.apiVersion)
	);

	if (app.apiVersion < MIN_VULKAN_VERSION) {
		throw std::runtime_error("Vulkan version is too old");
	}
	if (VK_API_VERSION_VARIANT(app.apiVersion) != 0) {
		throw std::runtime_error("Vulkan variant is not supported");
	}

	std::vector<const char*> instance_layers;
	std::vector<const char*> instance_extensions;

#ifndef NDEBUG
	instance_layers.push_back("VK_LAYER_KHRONOS_validation");
	instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

	auto info =
		vk::InstanceCreateInfo()
			.setPEnabledLayerNames(instance_layers)
			.setPEnabledExtensionNames(instance_extensions)
			.setPApplicationInfo(&app);

#ifndef NDEBUG
	auto debug =
		vk::DebugUtilsMessengerCreateInfoEXT()
			.setMessageSeverity(
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
				// | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
			)
			.setMessageType(
				vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
				| vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
			)
			.setPfnUserCallback(debug_callback)
			.setPUserData(this);

	info.setPNext(&debug);
#endif

	std::ignore = vk::createInstance(&info, nullptr, &m_instance);
	VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);

#ifndef NDEBUG
	std::ignore = m_instance.createDebugUtilsMessengerEXT(&debug, nullptr, &m_debug);
#endif
}

VulkanDevice::~VulkanDevice() {
#ifndef NDEBUG
	m_instance.destroyDebugUtilsMessengerEXT(m_debug);
#endif
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

#include <SDL3/SDL_vulkan.h>
#include <nvrhi/validation.h>
#include <nvrhi/vulkan.h>

#include <print>
#include <stdexcept>
#include <unordered_set>

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

#ifndef NDEBUG
	m_instance_layers.push_back("VK_LAYER_KHRONOS_validation");
	m_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

	u32 sdl_extension_count = 0;
	auto sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_extension_count);
	m_instance_extensions.append_range(std::span(sdl_extensions, sdl_extension_count));

	auto info =
		vk::InstanceCreateInfo()
			.setPEnabledLayerNames(m_instance_layers)
			.setPEnabledExtensionNames(m_instance_extensions)
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

	m_physical_device = m_instance.enumeratePhysicalDevices().front(); // lazy

	m_graphics_queue_index = m_compute_queue_index = m_transfer_queue_index = m_present_queue_index = 0; // lazy

	const std::unordered_set unique_queue_families = {
		m_graphics_queue_index,
		m_compute_queue_index,
		m_transfer_queue_index,
		m_present_queue_index,
	};

	std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
	queue_create_infos.reserve(unique_queue_families.size());

	for (int index : unique_queue_families) {
		constexpr float priority = 1.0f;
		queue_create_infos.push_back(
			vk::DeviceQueueCreateInfo().setQueueFamilyIndex(index).setQueueCount(1).setQueuePriorities(priority)
		);
	}

	vk::PhysicalDeviceFeatures device_features;
	device_features.samplerAnisotropy = true;

	vk::PhysicalDeviceVulkan12Features vk12_features;
	vk12_features.timelineSemaphore = true;

	auto device =
		vk::DeviceCreateInfo()
			.setQueueCreateInfos(queue_create_infos)
			.setPEnabledFeatures(&device_features)
			.setPEnabledExtensionNames(m_device_extensions)
			.setPNext(&vk12_features);

	std::ignore = m_physical_device.createDevice(&device, nullptr, &m_device);
	VULKAN_HPP_DEFAULT_DISPATCHER.init(m_device);

	m_device.getQueue(m_graphics_queue_index, 0, &m_graphics_queue);
	m_device.getQueue(m_compute_queue_index, 0, &m_compute_queue);
	m_device.getQueue(m_transfer_queue_index, 0, &m_transfer_queue);
	m_device.getQueue(m_present_queue_index, 0, &m_present_queue);

	nvrhi::vulkan::DeviceDesc desc;
	desc.errorCB = this;
	desc.instance = m_instance;
	desc.physicalDevice = m_physical_device;
	desc.device = m_device;

	desc.instanceExtensions = m_instance_extensions.data();
	desc.numInstanceExtensions = m_instance_extensions.size();
	desc.deviceExtensions = m_device_extensions.data();
	desc.numDeviceExtensions = m_device_extensions.size();

	desc.graphicsQueueIndex = m_graphics_queue_index;
	desc.computeQueueIndex = m_compute_queue_index;
	desc.transferQueueIndex = m_transfer_queue_index;
	desc.graphicsQueue = m_graphics_queue;
	desc.computeQueue = m_compute_queue;
	desc.transferQueue = m_transfer_queue;

	m_handle = nvrhi::vulkan::createDevice(desc);

#ifndef NDEBUG
	m_handle = nvrhi::validation::createValidationLayer(m_handle);
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

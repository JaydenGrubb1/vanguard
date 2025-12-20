#include <print>

#include "backends/dx12/device.hpp"
#include "gfx/device.hpp"

namespace vg::gfx {

std::unique_ptr<IDevice> IDevice::create() {
	return std::make_unique<DX12Device>();
}

void IDevice::message(const nvrhi::MessageSeverity severity, const char* text) {
	std::string_view level;

	switch (severity) {
		case nvrhi::MessageSeverity::Info:
			level = "Info";
			break;
		case nvrhi::MessageSeverity::Warning:
			level = "Warning";
			break;
		case nvrhi::MessageSeverity::Error:
			level = "Error";
			break;
		default:
			level = "Unknown";
			break;
	}

	std::println("[nvrhi][{}]: {}", level, text);
}

} // namespace vg::gfx

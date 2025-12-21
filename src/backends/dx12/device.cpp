#include <nvrhi/d3d12.h>
#include <nvrhi/validation.h>

#include <print>

#include "backends/dx12/device.hpp"

template<>
struct std::formatter<D3D12_MESSAGE_SEVERITY> : std::formatter<std::string_view> {
	template<typename FormatCtx>
	auto format(const D3D12_MESSAGE_SEVERITY severity, FormatCtx& ctx) const {
		switch (severity) {
			case D3D12_MESSAGE_SEVERITY_CORRUPTION:
				return std::formatter<std::string_view>::format("CORRUPTION", ctx);
			case D3D12_MESSAGE_SEVERITY_ERROR:
				return std::formatter<std::string_view>::format("ERROR", ctx);
			case D3D12_MESSAGE_SEVERITY_WARNING:
				return std::formatter<std::string_view>::format("WARNING", ctx);
			case D3D12_MESSAGE_SEVERITY_INFO:
				return std::formatter<std::string_view>::format("INFO", ctx);
			case D3D12_MESSAGE_SEVERITY_MESSAGE:
				return std::formatter<std::string_view>::format("MESSAGE", ctx);
			default:
				return std::formatter<std::string_view>::format("?", ctx);
		}
	}
};

namespace vg::gfx {

#ifndef NDEBUG
static void callback(D3D12_MESSAGE_CATEGORY, D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID, LPCSTR msg, void*) {
	if (severity <= D3D12_MESSAGE_SEVERITY_INFO)
		return;

	std::println("[d3d12][{}]: {}", severity, msg);
}
#endif

DX12Device::DX12Device() {
#ifdef NDEBUG
	std::ignore = CreateDXGIFactory2(0, IID_PPV_ARGS(&m_factory));
#else
	std::ignore = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_factory));

	nvrhi::RefCountPtr<ID3D12Debug3> debug;
	std::ignore = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
	debug->EnableDebugLayer();
	debug->SetEnableGPUBasedValidation(true);
#endif

	std::ignore = m_factory->EnumAdapters(0, &m_adapter);
	std::ignore = D3D12CreateDevice(m_adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));

#ifndef NDEBUG
	nvrhi::RefCountPtr<ID3D12InfoQueue1> info_queue;
	std::ignore = m_device->QueryInterface(IID_PPV_ARGS(&info_queue));
	std::ignore =
		info_queue->RegisterMessageCallback(callback, D3D12_MESSAGE_CALLBACK_IGNORE_FILTERS, this, &m_debug_cookie);

	if (info_queue) {
		// std::ignore = info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		std::ignore = info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		std::ignore = info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);

		std::array<D3D12_MESSAGE_ID, 0> disabled = {}; // TODO: add ignored messages

		D3D12_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.pIDList = disabled.data();
		filter.DenyList.NumIDs = static_cast<UINT>(disabled.size());

		std::ignore = info_queue->AddStorageFilterEntries(&filter);
	}
#endif

	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queue_desc.NodeMask = 1;

	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	std::ignore = m_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&m_graphics_queue));

	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	std::ignore = m_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&m_compute_queue));

	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
	std::ignore = m_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&m_transfer_queue));

	nvrhi::d3d12::DeviceDesc desc;
	desc.errorCB = this;
	desc.pDevice = m_device.Get();
	desc.pGraphicsCommandQueue = m_graphics_queue;
	desc.pComputeCommandQueue = m_compute_queue;
	desc.pCopyCommandQueue = m_transfer_queue;

	m_handle = nvrhi::d3d12::createDevice(desc);

#ifndef NDEBUG
	m_handle = nvrhi::validation::createValidationLayer(m_handle);
#endif
}

DX12Device::~DX12Device() {
	if (m_handle) {
		m_handle->waitForIdle();
		m_handle.Reset();
	}

#ifndef NDEBUG
	nvrhi::RefCountPtr<ID3D12InfoQueue1> info_queue;
	std::ignore = m_device->QueryInterface(IID_PPV_ARGS(&info_queue));
	std::ignore = info_queue->UnregisterMessageCallback(m_debug_cookie);
#endif
}

nvrhi::DeviceHandle DX12Device::handle() {
	return m_handle;
}

} // namespace vg::gfx

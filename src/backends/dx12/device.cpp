#include <nvrhi/d3d12.h>
#include <nvrhi/validation.h>

#include "backends/dx12/device.hpp"

namespace vg::gfx {

DX12Device::DX12Device() {
	std::ignore = CreateDXGIFactory2(0, IID_PPV_ARGS(&m_factory));
	std::ignore = m_factory->EnumAdapters(0, &m_adapter);
	std::ignore = D3D12CreateDevice(m_adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));

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
}

nvrhi::DeviceHandle DX12Device::handle() {
	return m_handle;
}

} // namespace vg::gfx

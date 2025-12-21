#pragma once

#include <d3d12.h>
#include <dxgi1_5.h>

#include "gfx/device.hpp"

namespace vg::gfx {

class DX12Device final : public IDevice {
  public:
	DX12Device();
	~DX12Device() override;

	nvrhi::DeviceHandle handle() override;

  private:
	nvrhi::DeviceHandle m_handle;

	nvrhi::RefCountPtr<IDXGIFactory2> m_factory;
	nvrhi::RefCountPtr<IDXGIAdapter> m_adapter;
	nvrhi::RefCountPtr<ID3D12Device> m_device;

	DWORD m_debug_cookie = 0;

	nvrhi::RefCountPtr<ID3D12CommandQueue> m_graphics_queue;
	nvrhi::RefCountPtr<ID3D12CommandQueue> m_compute_queue;
	nvrhi::RefCountPtr<ID3D12CommandQueue> m_transfer_queue;
};

} // namespace vg::gfx

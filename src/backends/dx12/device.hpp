#pragma once

#include <d3d12.h>
#include <dxgi1_5.h>

#include <vector>

#include "gfx/device.hpp"

namespace vg::gfx {

class DX12Device final : public IDevice {
  public:
	DX12Device();
	~DX12Device() override;

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

	nvrhi::RefCountPtr<IDXGIFactory2> m_factory;
	nvrhi::RefCountPtr<IDXGIAdapter> m_adapter;
	nvrhi::RefCountPtr<ID3D12Device> m_device;

	DWORD m_debug_cookie = 0;

	nvrhi::RefCountPtr<ID3D12CommandQueue> m_graphics_queue;
	nvrhi::RefCountPtr<ID3D12CommandQueue> m_compute_queue;
	nvrhi::RefCountPtr<ID3D12CommandQueue> m_transfer_queue;

	DXGI_SWAP_CHAIN_DESC1 m_swapchain_desc = {};
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC m_fullscreen_desc = {};

	nvrhi::RefCountPtr<IDXGISwapChain3> m_swapchain;
	nvrhi::RefCountPtr<ID3D12Fence> m_fence;

	std::vector<nvrhi::RefCountPtr<ID3D12Resource>> m_swapchain_buffers;
	std::vector<nvrhi::TextureHandle> m_swapchain_textures;
	std::vector<HANDLE> m_swapchain_events;

	u64 m_frame_count = 1;
};

} // namespace vg::gfx

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

void DX12Device::create_swapchain(SDL_Window* window) {
	const auto properties = SDL_GetWindowProperties(window);
	const auto handle =
		static_cast<HWND>(SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
	if (handle == nullptr)
		throw std::runtime_error("Failed to get window handle");

	RECT client_rect;
	GetClientRect(handle, &client_rect);
	const UINT width = client_rect.right - client_rect.left;
	const UINT height = client_rect.bottom - client_rect.top;

	std::memset(&m_swapchain_desc, 0, sizeof(DXGI_SWAP_CHAIN_DESC1));
	m_swapchain_desc.Width = width;
	m_swapchain_desc.Height = height;
	m_swapchain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // NOTE: Render target will use SRGB format
	m_swapchain_desc.SampleDesc.Count = 1;
	m_swapchain_desc.SampleDesc.Quality = 0;
	m_swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_swapchain_desc.BufferCount = 2;
	m_swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	std::memset(&m_fullscreen_desc, 0, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	m_fullscreen_desc.RefreshRate.Numerator = 60;
	m_fullscreen_desc.RefreshRate.Denominator = 1;
	m_fullscreen_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	m_fullscreen_desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	m_fullscreen_desc.Windowed = true;

	nvrhi::RefCountPtr<IDXGISwapChain1> swapchain;
	std::ignore =
		m_factory
			->CreateSwapChainForHwnd(m_graphics_queue, handle, &m_swapchain_desc, &m_fullscreen_desc, nullptr, &swapchain);
	std::ignore = swapchain->QueryInterface(IID_PPV_ARGS(&m_swapchain));

	create_render_targets();

	std::ignore = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));

	for (UINT i = 0; i < m_swapchain_desc.BufferCount; i++) {
		HANDLE event = CreateEvent(nullptr, false, true, nullptr);
		m_swapchain_events.push_back(event);
	}
}

void DX12Device::destroy_swapchain() {
	destroy_render_targets();

	for (HANDLE event : m_swapchain_events) {
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	m_swapchain_events.clear();

	if (m_swapchain) {
		std::ignore = m_swapchain->SetFullscreenState(false, nullptr);
	}

	m_fence.Reset();
	m_swapchain.Reset();
}

void DX12Device::resize_swapchain() {
	if (!m_handle) {
		return;
	}
	if (!m_swapchain) {
		return;
	}

	destroy_render_targets();

	HWND window;
	std::ignore = m_swapchain->GetHwnd(&window);

	RECT client_rect;
	GetClientRect(window, &client_rect);
	m_swapchain_desc.Width = client_rect.right - client_rect.left;
	m_swapchain_desc.Height = client_rect.bottom - client_rect.top;

	std::ignore = m_swapchain->ResizeBuffers(
		m_swapchain_desc.BufferCount,
		m_swapchain_desc.Width,
		m_swapchain_desc.Height,
		m_swapchain_desc.Format,
		m_swapchain_desc.Flags
	);

	create_render_targets();
}

void DX12Device::create_render_targets() {
	m_swapchain_buffers.resize(m_swapchain_desc.BufferCount);
	m_swapchain_textures.resize(m_swapchain_desc.BufferCount);

	for (UINT i = 0; i < m_swapchain_desc.BufferCount; i++) {
		std::ignore = m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_swapchain_buffers[i]));

		nvrhi::TextureDesc desc = {};
		desc.width = m_swapchain_desc.Width;
		desc.height = m_swapchain_desc.Height;
		desc.sampleCount = m_swapchain_desc.SampleDesc.Count;
		desc.sampleQuality = m_swapchain_desc.SampleDesc.Quality;
		desc.format = nvrhi::Format::SRGBA8_UNORM; // NOTE: Swapchain uses linear format
		desc.debugName = "swapchain_buffer";
		desc.isRenderTarget = true;
		desc.isUAV = false;
		desc.initialState = nvrhi::ResourceStates::Present;
		desc.keepInitialState = true;

		m_swapchain_textures[i] = m_handle->createHandleForNativeTexture(
			nvrhi::ObjectTypes::D3D12_Resource,
			nvrhi::Object(m_swapchain_buffers[i]),
			desc
		);
	}
}

void DX12Device::destroy_render_targets() {
	if (m_handle) {
		m_handle->waitForIdle();
		m_handle->runGarbageCollection();
	}

	for (HANDLE event : m_swapchain_events) {
		SetEvent(event);
	}

	m_swapchain_textures.clear();
	m_swapchain_buffers.clear();
}

void DX12Device::acquire_frame() {
	const UINT index = m_swapchain->GetCurrentBackBufferIndex();
	WaitForSingleObject(m_swapchain_events[index], INFINITE);
}

void DX12Device::present_frame() {
	const UINT index = m_swapchain->GetCurrentBackBufferIndex();
	std::ignore = m_swapchain->Present(1, 0);

	std::ignore = m_fence->SetEventOnCompletion(m_frame_count, m_swapchain_events[index]);
	std::ignore = m_graphics_queue->Signal(m_fence, m_frame_count);
	m_frame_count++;
}

u32 DX12Device::get_current_index() {
	return m_swapchain->GetCurrentBackBufferIndex();
}

u32 DX12Device::get_buffer_count() {
	return m_swapchain_desc.BufferCount;
}

nvrhi::TextureHandle DX12Device::get_buffer(const u32 index) {
	return m_swapchain_textures[index];
}

nvrhi::DeviceHandle DX12Device::get_device() {
	return m_handle;
}

} // namespace vg::gfx

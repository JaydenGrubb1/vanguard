#pragma once

#include <SDL3/SDL.h>
#include <nvrhi/nvrhi.h>

#include <memory>

#include "types.hpp"

namespace vg::gfx {

class IDevice : public nvrhi::IMessageCallback {
  public:
	static std::unique_ptr<IDevice> create();
	~IDevice() override = default;

	virtual void create_swapchain(SDL_Window* window) = 0;
	virtual void destroy_swapchain() = 0;
	virtual void resize_swapchain() = 0;

	virtual void create_render_targets() = 0;
	virtual void destroy_render_targets() = 0;

	virtual void acquire_frame() = 0;
	virtual void present_frame() = 0;

	virtual u32 get_current_index() = 0;
	virtual u32 get_buffer_count() = 0;
	virtual nvrhi::TextureHandle get_buffer(u32 index) = 0;
	virtual nvrhi::DeviceHandle get_device() = 0;

  private: // nvrhi::IMessageCallback
	void message(nvrhi::MessageSeverity severity, const char* text) override;
};

} // namespace vg::gfx

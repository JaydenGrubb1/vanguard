#pragma once

#include <list>
#include <type_traits>

#include "gfx/device.hpp"
#include "gfx/render_pass.hpp"

namespace vg::gfx {

class Renderer {
  public:
	explicit Renderer(IDevice& device);
	~Renderer() = default;

	template<typename Pass, typename... Args>
		requires std::derived_from<Pass, IRenderPass> && std::constructible_from<Pass, Args...>
	void add_render_pass(Args&&... args) {
		auto pass = std::make_unique<Pass>(std::forward<Args>(args)...);
		m_render_passes.push_back(std::move(pass));
	}

	void init() const;
	void render() const;

  private:
	IDevice& m_device;
	nvrhi::CommandListHandle m_command_list;

	std::list<std::unique_ptr<IRenderPass>> m_render_passes;
};

} // namespace vg::gfx

#pragma once

#include <list>
#include <type_traits>
#include <unordered_map>

#include "gfx/device.hpp"
#include "gfx/render_pass.hpp"

namespace vg::gfx {

class IOutputPass : public IRenderPass {
  public:
	virtual void set_framebuffer(nvrhi::IFramebuffer* framebuffer) = 0;
};

class RenderPassContext {
  public:
	void set_resource(const std::string& name, nvrhi::ResourceHandle resource);
	nvrhi::ResourceHandle get_resource(const std::string& name) const;

	template<typename T>
		requires std::derived_from<typename T::InterfaceType, nvrhi::IResource>
	T get_resource(const std::string& name) const {
		return T(static_cast<T::InterfaceType*>(get_resource(name).Get()));
	}

  private:
	std::unordered_map<std::string, nvrhi::ResourceHandle> m_resources;
};

class Renderer {
  public:
	explicit Renderer(IDevice& device);
	~Renderer() = default;

	template<typename Pass, typename... Args>
		requires std::derived_from<Pass, IRenderPass> && std::constructible_from<Pass, Args...>
	void add_render_pass(const RenderPassInfo& info, Args&&... args) {
		auto pass = std::make_unique<Pass>(std::forward<Args>(args)...);
		m_render_passes.emplace_back(info, std::move(pass));
	}

	void init() const;
	void render();

  private:
	IDevice& m_device;
	nvrhi::CommandListHandle m_command_list;

	std::list<std::pair<RenderPassInfo, std::unique_ptr<IRenderPass>>> m_render_passes;
	std::unique_ptr<IOutputPass> m_output_pass;
	RenderPassContext m_render_pass_context;
};

} // namespace vg::gfx

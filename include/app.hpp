#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <SDL3/SDL.h>

#include <span>
#include <string_view>

#include "gfx/device.hpp"
#include "types.hpp"

namespace vg {

struct Vertex {
	glm::vec3 pos;
	glm::vec2 uv;
};

class App {
  public:
	explicit App(std::span<const std::string_view> args);
	~App();

	void run();
	void quit();

  private:
	bool m_running = false;

	SDL_Window* m_window = nullptr;

	std::unique_ptr<gfx::IDevice> m_device;

	nvrhi::GraphicsPipelineHandle m_pipeline;
	nvrhi::CommandListHandle m_command_list;
	nvrhi::BindingSetHandle m_binding_set;

	std::vector<Vertex> m_vertices;
	std::vector<u32> m_indices;
	std::vector<glm::u8vec4> m_pixels;

	nvrhi::BufferHandle m_constant_buffer;
	nvrhi::BufferHandle m_vertex_buffer;
	nvrhi::BufferHandle m_index_buffer;
	
	nvrhi::TextureHandle m_texture;
	nvrhi::SamplerHandle m_sampler;
};

} // namespace vg

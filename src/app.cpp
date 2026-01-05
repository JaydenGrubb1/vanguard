#include <glm/mat4x4.hpp>

#include <nvrhi/utils.h>

#include <filesystem>
#include <fstream>
#include <print>
#include <ranges>
#include <stdexcept>

#include "app.hpp"

namespace vg {

static std::vector<std::byte> load_shader(const std::filesystem::path& path) {
	std::basic_ifstream<std::byte> file(path, std::ios::binary | std::ios::ate);
	if (!file)
		throw std::runtime_error("Failed to open file");

	const std::streamsize size = file.tellg();
	std::vector<std::byte> shader(size);

	file.seekg(0, std::ios::beg);
	file.read(shader.data(), size);
	file.close();

	return shader;
}

App::App(std::span<const std::string_view> args) {
	for (auto [idx, arg] : std::views::enumerate(args)) {
		std::println("arg[{}] = {}", idx, arg);
	}

	if (!SDL_Init(SDL_INIT_VIDEO))
		throw std::runtime_error("Failed to initialize SDL");

	m_window = SDL_CreateWindow("Vanguard", 1600, 900, SDL_WINDOW_RESIZABLE);
	if (m_window == nullptr)
		throw std::runtime_error("Failed to create window");

	std::println("current_path: {}", std::filesystem::current_path().string());

	auto vertex_shader_code = load_shader("shaders/basic.vs.dxil");
	auto fragment_shader_code = load_shader("shaders/basic.ps.dxil");

	m_device = gfx::IDevice::create();
	m_device->create_swapchain(m_window);
	m_device->resize_swapchain();

	auto vertex_shader = m_device->get_device()->createShader(
		nvrhi::ShaderDesc().setShaderType(nvrhi::ShaderType::Vertex),
		vertex_shader_code.data(),
		vertex_shader_code.size()
	);
	auto fragment_shader = m_device->get_device()->createShader(
		nvrhi::ShaderDesc().setShaderType(nvrhi::ShaderType::Pixel),
		fragment_shader_code.data(),
		fragment_shader_code.size()
	);

	m_vertices.push_back({{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}});
	m_vertices.push_back({{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}});
	m_vertices.push_back({{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}});
	m_vertices.push_back({{1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}});

	m_indices.push_back(0);
	m_indices.push_back(1);
	m_indices.push_back(2);
	m_indices.push_back(2);
	m_indices.push_back(3);
	m_indices.push_back(0);

	m_pixels.emplace_back(255, 0, 0, 255);
	m_pixels.emplace_back(0, 255, 0, 255);
	m_pixels.emplace_back(0, 0, 255, 255);
	m_pixels.emplace_back(255, 255, 0, 255);

	std::array<nvrhi::VertexAttributeDesc, 2> attributes = {
		nvrhi::VertexAttributeDesc()
			.setName("POSITION")
			.setFormat(nvrhi::Format::RGB32_FLOAT)
			.setOffset(offsetof(Vertex, pos))
			.setElementStride(sizeof(Vertex)),
		nvrhi::VertexAttributeDesc()
			.setName("TEXCOORD")
			.setFormat(nvrhi::Format::RG32_FLOAT)
			.setOffset(offsetof(Vertex, uv))
			.setElementStride(sizeof(Vertex)),
	};

	auto input_layout =
		m_device->get_device()->createInputLayout(attributes.data(), static_cast<u32>(attributes.size()), vertex_shader);

	nvrhi::FramebufferInfo framebuffer_info = {};
	framebuffer_info.addColorFormat(nvrhi::Format::SRGBA8_UNORM);

	nvrhi::BindingLayoutDesc layout_desc = {};
	layout_desc.setVisibility(nvrhi::ShaderType::All);
	layout_desc.addItem(nvrhi::BindingLayoutItem::VolatileConstantBuffer(0));
	layout_desc.addItem(nvrhi::BindingLayoutItem::Texture_SRV(0));
	layout_desc.addItem(nvrhi::BindingLayoutItem::Sampler(0));

	auto binding_layout = m_device->get_device()->createBindingLayout(layout_desc);

	nvrhi::GraphicsPipelineDesc pipeline_desc = {};
	pipeline_desc.setInputLayout(input_layout);
	pipeline_desc.setVertexShader(vertex_shader);
	pipeline_desc.setFragmentShader(fragment_shader);
	pipeline_desc.addBindingLayout(binding_layout);

	nvrhi::DepthStencilState ds = {};
	ds.depthTestEnable = false;
	ds.depthWriteEnable = false;
	ds.stencilEnable = false;

	pipeline_desc.renderState.setDepthStencilState(ds);

	m_pipeline = m_device->get_device()->createGraphicsPipeline(pipeline_desc, framebuffer_info);
	m_command_list = m_device->get_device()->createCommandList();

	nvrhi::BufferDesc constant_buffer_desc = {};
	constant_buffer_desc.setByteSize(sizeof(glm::mat4x4));
	constant_buffer_desc.setIsConstantBuffer(true);
	constant_buffer_desc.setIsVolatile(true);
	constant_buffer_desc.setMaxVersions(16);

	m_constant_buffer = m_device->get_device()->createBuffer(constant_buffer_desc);

	nvrhi::BufferDesc vertex_buffer_desc = {};
	vertex_buffer_desc.setByteSize(m_vertices.size() * sizeof(Vertex));
	vertex_buffer_desc.setIsVertexBuffer(true);
	vertex_buffer_desc.enableAutomaticStateTracking(nvrhi::ResourceStates::VertexBuffer); // what?
	vertex_buffer_desc.setDebugName("vertex_buffer");

	m_vertex_buffer = m_device->get_device()->createBuffer(vertex_buffer_desc);

	nvrhi::BufferDesc index_buffer_desc = {};
	index_buffer_desc.setByteSize(m_indices.size() * sizeof(u32));
	index_buffer_desc.setIsIndexBuffer(true);
	index_buffer_desc.enableAutomaticStateTracking(nvrhi::ResourceStates::IndexBuffer); // what?
	index_buffer_desc.setDebugName("index_buffer");

	m_index_buffer = m_device->get_device()->createBuffer(index_buffer_desc);

	nvrhi::TextureDesc texture_desc = {};
	texture_desc.setDimension(nvrhi::TextureDimension::Texture2D);
	texture_desc.setWidth(2); // dont hardcode
	texture_desc.setHeight(2); // dont hardcode
	texture_desc.setFormat(nvrhi::Format::SRGBA8_UNORM);
	texture_desc.enableAutomaticStateTracking(nvrhi::ResourceStates::ShaderResource); // what?
	texture_desc.setDebugName("texture");

	m_texture = m_device->get_device()->createTexture(texture_desc);

	nvrhi::SamplerDesc sampler_desc = {};
	sampler_desc.minFilter = false;
	sampler_desc.magFilter = false;

	m_sampler = m_device->get_device()->createSampler(sampler_desc);

	nvrhi::BindingSetDesc binding_set_desc = {};
	binding_set_desc.addItem(nvrhi::BindingSetItem::ConstantBuffer(0, m_constant_buffer));
	binding_set_desc.addItem(nvrhi::BindingSetItem::Texture_SRV(0, m_texture));
	binding_set_desc.addItem(nvrhi::BindingSetItem::Sampler(0, m_sampler));

	m_binding_set = m_device->get_device()->createBindingSet(binding_set_desc, binding_layout);

	// upload data to gpu
	m_command_list->open();
	m_command_list->writeBuffer(m_vertex_buffer, m_vertices.data(), m_vertices.size() * sizeof(Vertex));
	m_command_list->writeBuffer(m_index_buffer, m_indices.data(), m_indices.size() * sizeof(u32));
	m_command_list->writeTexture(m_texture, 0, 0, m_pixels.data(), 2 * sizeof(glm::u8vec4));
	m_command_list->close();
	m_device->get_device()->executeCommandList(m_command_list);
}

App::~App() {
	if (m_window != nullptr)
		SDL_DestroyWindow(m_window);

	SDL_Quit();
}

void App::run() {
	m_running = true;

	while (m_running) {
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_EVENT_QUIT:
					quit();
					break;
				case SDL_EVENT_WINDOW_RESIZED:
					m_device->resize_swapchain();
					break;
				default:
					break;
			}
		}

		// render frame
		auto framebuffer = m_device->begin_frame();
		const float width = static_cast<float>(framebuffer->getFramebufferInfo().width);
		const float height = static_cast<float>(framebuffer->getFramebufferInfo().height);

		m_command_list->open();

		nvrhi::utils::ClearColorAttachment(m_command_list, framebuffer, 0, nvrhi::Color(0.f));

		auto mvp = glm::mat4x4(1.0f);
		m_command_list->writeBuffer(m_constant_buffer, &mvp, sizeof(glm::mat4x4));

		nvrhi::GraphicsState state;
		state.setPipeline(m_pipeline);
		state.setFramebuffer(framebuffer);
		state.setViewport(nvrhi::ViewportState().addViewportAndScissorRect(nvrhi::Viewport(width, height)));
		state.addBindingSet(m_binding_set);

		state.setIndexBuffer({m_index_buffer, nvrhi::Format::R32_UINT, 0});
		state.addVertexBuffer({m_vertex_buffer, 0, offsetof(Vertex, pos)});
		state.addVertexBuffer({m_vertex_buffer, 1, offsetof(Vertex, uv)});

		m_command_list->setGraphicsState(state);
		m_command_list->drawIndexed(nvrhi::DrawArguments().setVertexCount(static_cast<u32>(m_indices.size())));

		m_command_list->close();

		m_device->get_device()->executeCommandList(m_command_list);
		m_device->end_frame();
	}
}

void App::quit() {
	m_running = false;
}

} // namespace vg

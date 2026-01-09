#include <fstream>

#include "gfx/shader.hpp"

namespace vg::gfx {

nvrhi::ShaderHandle
Shader::from_binary(const std::filesystem::path& path, const nvrhi::ShaderType type, nvrhi::IDevice* device) {
	std::basic_ifstream<std::byte> file(path, std::ios::in | std::ios::binary | std::ios::ate);
	if (!file)
		throw std::runtime_error("Failed to open shader file");

	const auto size = file.tellg();
	std::vector<std::byte> buffer(size);

	file.seekg(0, std::ios::beg);
	file.read(buffer.data(), size);
	file.close();

	return device->createShader(nvrhi::ShaderDesc().setShaderType(type), buffer.data(), buffer.size());
}

} // namespace vg::gfx

#pragma once

#include <nvrhi/nvrhi.h>

#include <filesystem>

namespace vg::gfx {

class Shader {
  public:
	static nvrhi::ShaderHandle
	from_binary(const std::filesystem::path& path, nvrhi::ShaderType type, nvrhi::IDevice* device);
};

} // namespace vg::gfx

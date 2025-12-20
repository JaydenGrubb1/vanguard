#pragma once

#include <nvrhi/nvrhi.h>

#include <memory>

namespace vg::gfx {

class IDevice : public nvrhi::IMessageCallback {
  public:
	static std::unique_ptr<IDevice> create();
	~IDevice() override = default;

	virtual nvrhi::DeviceHandle handle() = 0;

  private: // nvrhi::IMessageCallback
	void message(nvrhi::MessageSeverity severity, const char* text) override;
};

} // namespace vg::gfx

#pragma once
#include <private/include/CVulkanApplication.h>
#include <CRenderBackend.h>

namespace graphics_backend
{
	class CRenderBackend_Vulkan : public CRenderBackend
	{
	public:
		void Initialize(std::string const& appName, std::string const& engineName) override;
		void Release() override;
		void NewWindow(uint32_t width, uint32_t height, std::string const& windowName) override;
		bool AnyWindowRunning() override;
		void TickWindows() override;
	private:
		CVulkanApplication m_Application;
	};
}

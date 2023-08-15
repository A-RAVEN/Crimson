#pragma once
#include <private/include/CVulkanApplication.h>
#include <RenderInterface/header/CRenderBackend.h>

namespace graphics_backend
{
	class CRenderBackend_Vulkan : public CRenderBackend
	{
	public:
		void Initialize(std::string const& appName, std::string const& engineName) override;
		void InitializeThreadContextCount(CThreadManager* threadManager, uint32_t threadCount) override;
		void Release() override;
		std::shared_ptr<WindowHandle> NewWindow(uint32_t width, uint32_t height, std::string const& windowName) override;
		bool AnyWindowRunning() override;
		void TickWindows() override;
		virtual void StartCurrentFrame() override;
		virtual void EndCurrentFrame() override;
		virtual void ExecuteRenderPass(CRenderpassBuilder const& inRenderPass) override;
		virtual void ExecuteRenderGraph(std::shared_ptr<CRenderGraph> inRenderGraph) override;
		virtual std::shared_ptr<GPUBuffer> CreateGPUBuffer(EBufferUsageFlags usageFlags
			, uint64_t count
			, uint64_t stride) override;
	private:
		CVulkanApplication m_Application;
	};
}

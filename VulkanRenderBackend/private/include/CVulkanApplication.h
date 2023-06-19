#pragma once
#include <private/include/WindowContext.h>
#include <private/include/CVulkanThreadContext.h>

namespace graphics_backend
{
	class CVulkanApplication
	{
	public:
		//friend class ApplicationSubobjectBase;
		~CVulkanApplication();
		void InitApp(std::string const& appName, std::string const& engineName);
		void ReleaseApp();
		inline vk::Instance GetInstance() const
		{
			return m_Instance;
		}
		inline vk::Device GetDevice() const
		{
			return m_Device;
		}
		bool AnyWindowRunning() const { return !m_WindowContexts.empty(); }
		void CreateWindowContext(std::string windowName, uint32_t initialWidth, uint32_t initialHeight);
		void TickWindowContexts();
	private:
		void InitializeInstance(std::string const& name, std::string const& engineName);
		void DestroyInstance();
		void EnumeratePhysicalDevices();
		void CreateDevice();
		void DestroyDevice();

		void InitializeThreadContext(uint32_t threadCount);
		void DestroyThreadContexts();

		void ReleaseAllWindowContexts();
		CVulkanThreadContext& GetThreadContext(uint32_t threadKey) const;
	private:
		vk::Instance m_Instance = nullptr;
		vk::PhysicalDevice m_PhysicalDevice = nullptr;
		vk::Device m_Device = nullptr;
	#if !defined(NDEBUG)
		vk::DebugUtilsMessengerEXT m_DebugMessager = nullptr;
	#endif

		std::vector<CWindowContext> m_WindowContexts;
		std::vector<CVulkanThreadContext> m_ThreadContexts;
	private:
		uint32_t m_LastSubmitFrame = 0;
	};
}

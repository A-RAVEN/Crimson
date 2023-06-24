#pragma once
#include <private/include/WindowContext.h>
#include <private/include/CVulkanThreadContext.h>
#include <private/include/FrameCountContext.h>

namespace graphics_backend
{
	class CVulkanApplication
	{
	public:
		//friend class ApplicationSubobjectBase;
		~CVulkanApplication();
		void InitApp(std::string const& appName, std::string const& engineName);
		void InitializeThreadContext(uint32_t threadCount);
		void ReleaseApp();
		inline vk::Instance const& GetInstance() const
		{
			return m_Instance;
		}
		inline vk::Device const& GetDevice() const
		{
			return m_Device;
		}
		inline vk::PhysicalDevice const& GetPhysicalDevice() const
		{
			return m_PhysicalDevice;
		}
		inline CVulkanThreadContext& GetThreadContext(uint32_t threadKey) const {
			return m_ThreadContexts[threadKey];
		}
		bool AnyWindowRunning() const { return !m_WindowContexts.empty(); }
		void CreateWindowContext(std::string windowName, uint32_t initialWidth, uint32_t initialHeight);
		void TickWindowContexts();

		CFrameCountContext const& GetSubmitCounterContext() const { return m_SubmitCounterContext; }

		template<typename T>
		T SubObject() const {
			static_assert(std::is_base_of<ApplicationSubobjectBase, T>::value, "Type T not derived from ApplicationSubobjectBase");
			T newSubObject;
			newSubObject.Initialize(this);
			return newSubObject;
		}

		void ReleaseSubObject(ApplicationSubobjectBase& subobject) const
		{
			subobject.Release();
		}

		void TickRunTest();
	private:

		void InitializeInstance(std::string const& name, std::string const& engineName);
		void DestroyInstance();
		void EnumeratePhysicalDevices();
		void CreateDevice();
		void DestroyDevice();

		void DestroyThreadContexts();

		void ReleaseAllWindowContexts();
	private:
		vk::Instance m_Instance = nullptr;
		vk::PhysicalDevice m_PhysicalDevice = nullptr;
		vk::Device m_Device = nullptr;
	#if !defined(NDEBUG)
		vk::DebugUtilsMessengerEXT m_DebugMessager = nullptr;
	#endif

		CFrameCountContext m_SubmitCounterContext;
		std::vector<CWindowContext> m_WindowContexts;
		mutable std::vector<CVulkanThreadContext> m_ThreadContexts;
	private:
		
		uint32_t m_LastSubmitFrame = 0;
	};
}

#pragma once
#include <private/include/WindowContext.h>
#include <private/include/CVulkanThreadContext.h>
#include <private/include/FrameCountContext.h>
#include <ThreadManager/header/ThreadManger.h>

namespace graphics_backend
{
	using namespace thread_management;
	class CVulkanApplication
	{
	public:
		//friend class ApplicationSubobjectBase;
		~CVulkanApplication();
		void InitApp(std::string const& appName, std::string const& engineName);
		void InitializeThreadContext(CThreadManager* threadManager, uint32_t threadCount);
		void ReleaseApp();
		void DeviceWaitIdle();
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
		inline CVulkanThreadContext* GetThreadContext(uint32_t threadKey) const {
			if (threadKey >= m_ThreadContexts.size())
			{
				return nullptr;
			}
			return &m_ThreadContexts[threadKey];
		}
		CVulkanThreadContext& AquireThreadContext();
		CThreadManager* GetThreadManager() const;
		void ReturnThreadContext(CVulkanThreadContext& returningContext);
		bool AnyWindowRunning() const { return !m_WindowContexts.empty(); }
		void CreateWindowContext(std::string windowName, uint32_t initialWidth, uint32_t initialHeight);
		void TickWindowContexts();

		void TestEnqueueBufferLoadingTask(CThreadManager* pThreadManger);

		CFrameCountContext const& GetSubmitCounterContext() const { return m_SubmitCounterContext; }

		template<typename T, typename...TArgs>
		T SubObject(TArgs&&...Args) const {
			static_assert(std::is_base_of<ApplicationSubobjectBase, T>::value, "Type T not derived from ApplicationSubobjectBase");
			T newSubObject( std::forward<TArgs>(Args)... );
			newSubObject.Initialize(this);
			return newSubObject;
		}

		template<typename T, typename...TArgs>
		T& SubObject_EmplaceBack(std::vector<T>& container,TArgs&&...Args) const {
			static_assert(std::is_base_of<ApplicationSubobjectBase, T>::value, "Type T not derived from ApplicationSubobjectBase");
			container.emplace_back(std::forward<TArgs>(Args)...);
			T& newSubObject = container.back();
			newSubObject.Initialize(this);
			return newSubObject;
		}

		template<typename T, typename...TArgs>
		std::shared_ptr<T> SubObject_Shared(TArgs&&...Args) const {
			static_assert(std::is_base_of<ApplicationSubobjectBase, T>::value, "Type T not derived from ApplicationSubobjectBase");
			std::shared_ptr<T> newSubObject = std::shared_ptr<T>(new T(std::forward<TArgs>(Args)...), ApplicationSubobjectBase_Deleter{});
			newSubObject->Initialize(this);
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

		Internal_InterlockedQueue<uint32_t> m_AvailableThreadQueue;
		mutable std::vector<CVulkanThreadContext> m_ThreadContexts;
		CThreadManager* p_ThreadManager = nullptr;
	};
}

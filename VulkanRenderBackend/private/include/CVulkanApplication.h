#pragma once
#include <private/include/WindowContext.h>
#include <private/include/CVulkanThreadContext.h>
#include <private/include/FrameCountContext.h>
#include <private/include/CVulkanMemoryManager.h>
#include <ThreadManager/header/ThreadManager.h>
#include <private/include/CPrimitiveResource_Vulkan.h>
#include <private/include/Containers.h>
#include <unordered_map>

#include "CShaderModuleObject.h"
#include "RenderPassObject.h"
#include "VulkanPipelineObject.h"
#include "FramebufferObject.h"
#include "GPUBuffer_Impl.h"

namespace graphics_backend
{
	using namespace thread_management;

	class CVulkanApplication
	{
	public:
		CVulkanApplication();
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
		CVulkanMemoryManager& GetMemoryManager() const;
		CVulkanThreadContext& AquireThreadContext();
		CThreadManager* GetThreadManager() const;
		CTask* NewTask();
		void ReturnThreadContext(CVulkanThreadContext& returningContext);
		bool AnyWindowRunning() const { return !m_WindowContexts.empty(); }
		void CreateWindowContext(std::string windowName, uint32_t initialWidth, uint32_t initialHeight);
		void TickWindowContexts();

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

		void PrepareBeforeTick();
		void EndThisFrame();
		void RunGraphWithPresentTarget(std::string const& targetName);

		void CreateImageViews2D(vk::Format format, std::vector<vk::Image> const& inImages, std::vector<vk::ImageView>& outImageViews) const;

		void TestCode();

		void ExecuteRenderPass(CRenderpassBuilder const& inRenderPass);
	public:
		CGPUPrimitiveResource_Vulkan* NewPrimitiveResource();
		void DestroyPrimitiveResource(CGPUPrimitiveResource_Vulkan*);
	public:
		GPUBuffer* NewGPUBuffer(EBufferUsageFlags usageFlags, uint64_t count, uint64_t stride);
		void ReleaseGPUBuffer(GPUBuffer* releaseGPUBuffer);
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
		CTaskGraph* p_TaskGraph = nullptr;
		CTask* p_RootTask = nullptr;
		std::shared_future<void> m_TaskFuture;

		TThreadSafePointerPool<CGPUPrimitiveResource_Vulkan> m_PrimitiveResourcePool;
		TThreadSafePointerPool<GPUBuffer_Impl> m_GPUBufferPool;

		ShaderModuleObjectDic m_ShaderModuleCache;
		RenderPassObjectDic m_RenderPassCache;
		PipelineObjectDic m_PipelineObjectCache;
		FramebufferObjectDic m_FramebufferObjectCache;

		mutable CVulkanMemoryManager m_MemoryManager;
	};
}

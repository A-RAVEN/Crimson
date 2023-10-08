#pragma once
#include <private/include/WindowContext.h>
#include <private/include/CVulkanThreadContext.h>
#include <private/include/FrameCountContext.h>
#include <private/include/CVulkanMemoryManager.h>
#include <ThreadManager/header/ThreadManager.h>
#include <private/include/Containers.h>
#include <unordered_map>

#include "CShaderModuleObject.h"
#include "RenderPassObject.h"
#include "VulkanPipelineObject.h"
#include "FramebufferObject.h"
#include "GPUBuffer_Impl.h"
#include <RenderInterface/header/CRenderGraph.h>
#include <RenderInterface/header/ShaderBindingBuilder.h>
#include "GPUObjectManager.h"
#include "RenderGraphExecutor.h"
#include "ShaderBindingSet_Impl.h"
#include "GPUTexture_Impl.h"

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
		GPUObjectManager& GetGPUObjectManager() { return m_GPUObjectManager; }
		CVulkanMemoryManager& GetMemoryManager();

		CVulkanThreadContext& AquireThreadContext();
		void ReturnThreadContext(CVulkanThreadContext& returningContext);
		std::shared_ptr<CVulkanThreadContext> AquireThreadContextPtr();

		CThreadManager* GetThreadManager() const;
		CTask* NewTask();
		TaskParallelFor* NewTaskParallelFor();
		CTask* NewUploadingTask(UploadingResourceType resourceType);
		bool AnyWindowRunning() const { return !m_WindowContexts.empty(); }
		std::shared_ptr<WindowHandle> CreateWindowContext(std::string windowName, uint32_t initialWidth, uint32_t initialHeight);
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

		template<typename T, typename...TArgs>
		T NewObject(TArgs&&...Args) const {
			static_assert(std::is_base_of<BaseApplicationSubobject, T>::value, "Type T not derived from BaseApplicationSubobject");
			T newObject(*this);
			newObject.Initialize(std::forward<TArgs>(Args)...);
			return newObject;
		}

		void ReleaseSubObject(ApplicationSubobjectBase& subobject) const
		{
			subobject.Release();
		}

		void PrepareBeforeTick();
		void EndThisFrame();
		void ExecuteRenderGraph(std::shared_ptr<CRenderGraph> inRenderGraph);

		void CreateImageViews2D(vk::Format format, std::vector<vk::Image> const& inImages, std::vector<vk::ImageView>& outImageViews) const;
		vk::ImageView CreateDefaultImageView(
			GPUTextureDescriptor const& inDescriptor
			, vk::Image inImage
			, bool depthAspect
			, bool stencilAspect) const;
	public:
		//Allocation
		GPUBuffer* NewGPUBuffer(EBufferUsageFlags usageFlags, uint64_t count, uint64_t stride);
		void ReleaseGPUBuffer(GPUBuffer* releaseGPUBuffer);

		GPUTexture* NewGPUTexture(GPUTextureDescriptor const& inDescriptor);
		void ReleaseGPUTexture(GPUTexture* releaseGPUTexture);

		std::shared_ptr<ShaderConstantSet> NewShaderConstantSet(ShaderConstantsBuilder const& builder);
		std::shared_ptr<ShaderBindingSet> NewShaderBindingSet(ShaderBindingBuilder const& builder);
		std::shared_ptr<TextureSampler> GetOrCreateTextureSampler(TextureSamplerDescriptor const& descriptor);

		HashPool<ShaderBindingBuilder, ShaderBindingSetAllocator>& GetShaderBindingSetAllocators() { return m_ShaderBindingSetAllocator; }
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
		std::vector<std::shared_ptr<CWindowContext>> m_WindowContexts;

		Internal_InterlockedQueue<uint32_t> m_AvailableThreadQueue;
		mutable std::vector<CVulkanThreadContext> m_ThreadContexts;

		//Thread Manager
		CThreadManager* p_ThreadManager = nullptr;
		//Root Graph
		CTaskGraph* p_TaskGraph = nullptr;
		//Resource Loading Graph
		CTaskGraph* p_MemoryResourceUploadingTaskGraph = nullptr;
		CTaskGraph* p_GPUAddressUploadingTaskGraph = nullptr;
		//Rendergraph Execution Graph
		CTaskGraph* p_RenderingTaskGraph = nullptr;
		//Finalize Graph
		CTaskGraph* p_FinalizeTaskGraph = nullptr;
		//Future
		std::shared_future<void> m_TaskFuture;

		TVulkanApplicationPool<GPUBuffer_Impl> m_GPUBufferPool;
		TVulkanApplicationPool<GPUTexture_Impl> m_GPUTexturePool;
		//Uniform Buffer
		HashPool<ShaderConstantsBuilder, ShaderConstantSetAllocator> m_ConstantSetAllocator;
		//Shader Descriptor Set
		HashPool<ShaderBindingBuilder, ShaderBindingSetAllocator> m_ShaderBindingSetAllocator;

		GPUObjectManager m_GPUObjectManager;
		RenderGraphExecutorDic m_RenderGraphDic;
		//std::vector<RenderGraphExecutor*> m_CurrentFrameRenderGraphExecutors;

		std::deque<RenderGraphExecutor> m_Executors;

		CVulkanMemoryManager m_MemoryManager;
	};
}

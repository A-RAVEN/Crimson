#include "private/include/pch.h"
#include "private/include/Utils.h"
#include <private/include/RenderBackendSettings.h>
#include "private/include/CVulkanApplication.h"
#include "private/include/CVulkanThreadContext.h"
#include <private/include/CVulkanBufferObject.h>
#include <private/include/VulkanBarrierCollector.h>
#include <ShaderCompiler/header/Compiler.h>
#include <SharedTools/header/library_loader.h>
#include <SharedTools/header/FileLoader.h>
#include <private/test/TestShaderProvider.h>
#include <private/include/CommandList_Impl.h>
#include <private/include/InterfaceTranslator.h>
#include <private/include/RenderGraphExecutor.h>

namespace graphics_backend
{
	void CVulkanApplication::PrepareBeforeTick()
	{
		p_TaskGraph = p_ThreadManager->NewTaskGraph();
		p_TaskGraph->Name("GPU Task Graph");

		p_RootTask = p_TaskGraph->NewTask()
			->Name("GPU Frame Initialize")
			->Functor([this]()
				{
					m_SubmitCounterContext.WaitingForCurrentFrame();
					if (m_SubmitCounterContext.AnyFrameFinished())
					{
						FrameType const releasedFrame = m_SubmitCounterContext.GetReleasedFrameID();
						for (auto itrThreadContext = m_ThreadContexts.begin(); itrThreadContext != m_ThreadContexts.end(); ++itrThreadContext)
						{
							itrThreadContext->DoReleaseResourceBeforeFrame(releasedFrame);
						}
					}
					m_MemoryManager.ReleaseCurrentFrameResource();
					for (auto& context : m_WindowContexts)
					{
						context->WaitCurrentFrameBufferIndex();
					}
				});
	}

	void CVulkanApplication::EndThisFrame()
	{
		p_TaskGraph->FinalizeFunctor([this]()
			{
				std::vector<vk::CommandBuffer> waitingSubmitCommands;
				for (auto itrThreadContext = m_ThreadContexts.begin(); itrThreadContext != m_ThreadContexts.end(); ++itrThreadContext)
				{
					itrThreadContext->CollectSubmittingCommandBuffers(waitingSubmitCommands);
				}

				if(!m_WindowContexts.empty())
				{
					TIndex currentBuffer = m_WindowContexts[0]->GetCurrentFrameBufferIndex();

					std::array<const vk::Semaphore, 1> semaphore = { m_WindowContexts[0]->m_WaitNextFrameSemaphore };
					std::array<const vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eTransfer };
					std::array<const vk::Semaphore, 1> presentSemaphore = { m_WindowContexts[0]->m_CanPresentSemaphore };

					auto& threadContext0 = m_ThreadContexts[0];
					auto cmd = threadContext0.GetCurrentFramePool().AllocateOnetimeCommandBuffer();

					VulkanBarrierCollector presentBarrier{ GetSubmitCounterContext().GetGraphicsQueueFamily() };
					presentBarrier.PushImageBarrier(m_WindowContexts[0]->GetCurrentFrameImage()
						, ResourceUsage::eColorAttachmentOutput, ResourceUsage::ePresent);
					presentBarrier.ExecuteBarrier(cmd);

					cmd.end();
					waitingSubmitCommands.push_back(cmd);

					m_SubmitCounterContext.FinalizeCurrentFrameGraphics(waitingSubmitCommands
						, semaphore
						, waitStages
						, presentSemaphore);

					vk::PresentInfoKHR presenttInfo(
						presentSemaphore
						, m_WindowContexts[0]->m_Swapchain
						, currentBuffer
					);
					m_WindowContexts[0]->m_PresentQueue.second.presentKHR(presenttInfo);
				}
				else
				{
					m_SubmitCounterContext.FinalizeCurrentFrameGraphics(waitingSubmitCommands);
				}
			});

		if (m_TaskFuture.valid())
		{
			m_TaskFuture.wait();
		}

		m_TaskFuture = p_ThreadManager->ExecuteTaskGraph(p_TaskGraph);
	}

	void CVulkanApplication::ExecuteRenderGraph(std::shared_ptr<CRenderGraph> inRenderGraph)
	{
		std::shared_ptr<RenderGraphExecutor> executor = m_RenderGraphDic.GetOrCreate(inRenderGraph).lock();
		executor->Run();
	}

	void CVulkanApplication::CreateImageViews2D(vk::Format format, std::vector<vk::Image> const& inImages,
		std::vector<vk::ImageView>& outImageViews) const
	{
		for(auto& img : inImages)
		{
			vk::ImageViewCreateInfo createInfo({}
				, img
				, vk::ImageViewType::e2D
				, format
				, vk::ComponentMapping(
					vk::ComponentSwizzle::eIdentity
					, vk::ComponentSwizzle::eIdentity
					, vk::ComponentSwizzle::eIdentity
					, vk::ComponentSwizzle::eIdentity)
				, vk::ImageSubresourceRange(
					vk::ImageAspectFlagBits::eColor
					, 0
					, 1
					, 0
					, 1));
			auto newView = GetDevice().createImageView(createInfo);
			outImageViews.push_back(newView);
		}
	}

	GPUBuffer* CVulkanApplication::NewGPUBuffer(EBufferUsageFlags usageFlags, uint64_t count, uint64_t stride)
	{
		GPUBuffer_Impl* result = m_GPUBufferPool.Alloc(usageFlags, count, stride);
		return result;
	}

	void CVulkanApplication::ReleaseGPUBuffer(GPUBuffer* releaseGPUBuffer)
	{
		m_GPUBufferPool.Release(static_cast<GPUBuffer_Impl*>(releaseGPUBuffer));
	}

	void CVulkanApplication::InitializeInstance(std::string const& name, std::string const& engineName)
	{
		vk::ApplicationInfo application_info(
			name.c_str()
			, 1
			, engineName.c_str()
			, 0
			, VULKAN_API_VERSION_IN_USE);

		std::array<const char*, 1> extensionNames = {
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME
		};

		const std::vector<const char*> g_validationLayers{
			"VK_LAYER_KHRONOS_validation"
		};


		auto extensions = GetInstanceExtensionNames();
		vk::InstanceCreateInfo instance_info({}, &application_info, g_validationLayers, extensions);

#if !defined(NDEBUG)
		vk::DebugUtilsMessengerCreateInfoEXT debugUtilsExt = vulkan_backend::utils::makeDebugUtilsMessengerCreateInfoEXT();
		instance_info.setPNext(&debugUtilsExt);
#endif
		m_Instance = vk::createInstance(instance_info);

		vulkan_backend::utils::SetupVulkanInstanceFunctionPointers(m_Instance);
	#if !defined(NDEBUG)
		m_DebugMessager = m_Instance.createDebugUtilsMessengerEXT(debugUtilsExt);
	#endif
	}

	void CVulkanApplication::DestroyInstance()
	{
		if (m_Instance != vk::Instance(nullptr))
		{
	#if !defined( NDEBUG )
			m_Instance.destroyDebugUtilsMessengerEXT(m_DebugMessager);
			m_DebugMessager = nullptr;
	#endif

			vulkan_backend::utils::CleanupVulkanInstanceFuncitonPointers();
			m_Instance.destroy();
			m_Instance = nullptr;
		}
	}

	void CVulkanApplication::EnumeratePhysicalDevices()
	{
		m_PhysicalDevice = m_Instance.enumeratePhysicalDevices().front();
	}

	void CVulkanApplication::CreateDevice()
	{
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_PhysicalDevice.getQueueFamilyProperties();

		std::vector<std::pair<uint32_t, uint32_t>> generalUsageQueues;
		std::vector<std::pair<uint32_t, uint32_t>> computeDedicateQueues;
		std::vector<std::pair<uint32_t, uint32_t>> transferDedicateQueues;

		vk::QueueFlags generalFlags = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer;
		for (uint32_t queueId = 0; queueId < queueFamilyProperties.size(); ++queueId)
		{
			vk::QueueFamilyProperties const& itrProp = queueFamilyProperties[queueId];
			if ((itrProp.queueFlags & generalFlags) == generalFlags)
			{
				generalUsageQueues.push_back(std::make_pair(queueId, itrProp.queueCount));
			}
			else
			{
				if (itrProp.queueFlags & vk::QueueFlagBits::eCompute)
				{
					computeDedicateQueues.push_back(std::make_pair(queueId, itrProp.queueCount));
				}
				else if (itrProp.queueFlags & vk::QueueFlagBits::eTransfer)
				{
					transferDedicateQueues.push_back(std::make_pair(queueId, itrProp.queueCount));
				}
				else
				{
				}
			}
		}

		CA_ASSERT(!generalUsageQueues.empty(), "Vulkan: No General Usage Queue Found!");

		if (computeDedicateQueues.empty())
		{
			computeDedicateQueues.push_back(generalUsageQueues[0]);
		}
		if (transferDedicateQueues.empty())
		{
			transferDedicateQueues.push_back(generalUsageQueues[0]);
		}
		std::set<std::pair<uint32_t, uint32_t>> queueFamityIndices;
		queueFamityIndices.insert(generalUsageQueues[0]);
		queueFamityIndices.insert(computeDedicateQueues[0]);
		queueFamityIndices.insert(transferDedicateQueues[0]);

		std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfoList;
		std::pair<uint32_t, uint32_t> generalQueueRef;
		std::pair<uint32_t, uint32_t> computeQueueRef;
		std::pair<uint32_t, uint32_t> transferQueueRef;
		std::vector<std::vector<float>> queuePriorities;
		for (std::pair<uint32_t, uint32_t> const& itrQueueInfo : queueFamityIndices)
		{
			uint32_t queueFamilyId = itrQueueInfo.first;
			uint32_t queueCount = itrQueueInfo.second;

			uint32_t itrQueueId = 0;
			uint32_t requiredQueueCount = 0;
			if (queueFamilyId == generalUsageQueues[0].first)
			{
				generalQueueRef = std::make_pair(queueFamilyId, itrQueueId);
				itrQueueId = (itrQueueId + 1) % queueCount;
				requiredQueueCount = std::min(requiredQueueCount + 1, queueCount);
			}
			if (queueFamilyId == computeDedicateQueues[0].first)
			{
				computeQueueRef = std::make_pair(queueFamilyId, itrQueueId);
				itrQueueId = (itrQueueId + 1) % queueCount;
				requiredQueueCount = std::min(requiredQueueCount + 1, queueCount);
			}

			if (queueFamilyId == transferDedicateQueues[0].first)
			{
				transferQueueRef = std::make_pair(queueFamilyId, itrQueueId);
				itrQueueId = (itrQueueId + 1) % queueCount;
				requiredQueueCount = std::min(requiredQueueCount + 1, queueCount);
			}

			{
				std::vector<float> tmp;
				tmp.resize(requiredQueueCount);
				std::fill(tmp.begin(), tmp.end(), 0.0f);
				queuePriorities.push_back(tmp);
			}
			auto& currentQueuePriorities = queuePriorities.back();
			deviceQueueCreateInfoList.emplace_back(vk::DeviceQueueCreateFlags(), queueFamilyId, currentQueuePriorities);
		}

		std::vector<uint32_t> otherQueueFamilies;
		for(uint32_t familyId = 0; familyId < queueFamilyProperties.size(); ++familyId)
		{
			if(familyId != generalQueueRef.first && familyId != computeQueueRef.first && familyId != transferQueueRef.first)
			{
				otherQueueFamilies.push_back(familyId);
			}
		}


		for(uint32_t itrOtherFamilyId : otherQueueFamilies)
		{
			queuePriorities.push_back({ 0.0f });
			auto& currentQueuePriorities = queuePriorities.back();
			deviceQueueCreateInfoList.emplace_back(vk::DeviceQueueCreateFlags(), itrOtherFamilyId, currentQueuePriorities);
		}

		auto extensions = GetDeviceExtensionNames();
		vk::DeviceCreateInfo deviceCreateInfo({}, deviceQueueCreateInfoList, {}, extensions);

		m_Device = m_PhysicalDevice.createDevice(deviceCreateInfo);

		std::vector<vk::Queue> defaultQueues;
		defaultQueues.reserve(queueFamilyProperties.size());
		for(int itrFamily = 0; itrFamily < queueFamilyProperties.size(); ++itrFamily)
		{
			vk::Queue itrQueue = m_Device.getQueue(itrFamily, 0);
			defaultQueues.push_back(itrQueue);
		}

		m_SubmitCounterContext.Initialize(this);
		m_SubmitCounterContext.InitializeSubmitQueues(generalQueueRef, computeQueueRef, transferQueueRef);
		m_SubmitCounterContext.InitializeDefaultQueues(defaultQueues);

		vulkan_backend::utils::SetupVulkanDeviceFunctinoPointers(m_Device);
	}

	void CVulkanApplication::DestroyDevice()
	{
		m_SubmitCounterContext.Release();
		if(m_Device != vk::Device(nullptr))
		{
			m_Device.destroy();
			m_Device = nullptr;
		}
	}

	void CVulkanApplication::InitializeThreadContext(CThreadManager* threadManager, uint32_t threadCount)
	{
		CA_ASSERT(threadCount > 0, "Thread Count Should Be Greater Than 0");
		CA_ASSERT(m_ThreadContexts.size() == 0, "Thread Contexts Are Already Initialized");
		p_ThreadManager = threadManager;
		m_ThreadContexts.reserve(threadCount);
		for (uint32_t threadContextId = 0; threadContextId < threadCount; ++threadContextId)
		{
			m_ThreadContexts.push_back(SubObject<CVulkanThreadContext>(threadContextId));
		}
		std::vector<uint32_t> threadInitializeValue;
		threadInitializeValue.resize(threadCount);
		uint32_t id = 0;
		std::generate(threadInitializeValue.begin(), threadInitializeValue.end(), [&id]()
			{
				return id++;
			});
		m_AvailableThreadQueue.Initialize(threadInitializeValue);
	}

	void CVulkanApplication::DestroyThreadContexts()
	{
		for (auto& threadContext : m_ThreadContexts)
		{
			ReleaseSubObject(threadContext);
		}
		m_ThreadContexts.clear();
	}

	CVulkanMemoryManager& CVulkanApplication::GetMemoryManager()
	{
		return m_MemoryManager;
	}

	CVulkanThreadContext& CVulkanApplication::AquireThreadContext()
	{
		uint32_t available = m_AvailableThreadQueue.TryGetFront();
		return m_ThreadContexts[available];
	}

	CThreadManager* CVulkanApplication::GetThreadManager() const
	{
		return p_ThreadManager;
	}

	CTask* CVulkanApplication::NewTask()
	{
		CTask* newTask = p_TaskGraph->NewTask();
		newTask->Succeed(p_RootTask);
		return newTask;
	}

	void CVulkanApplication::ReturnThreadContext(CVulkanThreadContext& returningContext)
	{
		uint32_t id = returningContext.GetThreadID();
		m_AvailableThreadQueue.Enqueue(id);
	}

	std::shared_ptr<WindowHandle> CVulkanApplication::CreateWindowContext(std::string windowName, uint32_t initialWidth, uint32_t initialHeight)
	{
		m_WindowContexts.emplace_back(std::make_shared<CWindowContext>(*this));
		auto newContext = m_WindowContexts.back();
		newContext->Initialize(windowName, initialWidth, initialHeight);
		return newContext;
	}

	void CVulkanApplication::TickWindowContexts()
	{
		bool anyNeedClose = std::any_of(m_WindowContexts.begin(), m_WindowContexts.end(), [](auto& wcontest)
			{
				return wcontest->NeedClose();
			});
		if(anyNeedClose)
		{
			DeviceWaitIdle();
			size_t lastIndex = m_WindowContexts.size();
			size_t currentIndex = 0;
			while (currentIndex < m_WindowContexts.size())
			{
				if (m_WindowContexts[currentIndex]->NeedClose())
				{
					m_WindowContexts[currentIndex]->Release();
					std::swap(m_WindowContexts[currentIndex], m_WindowContexts.back());
					m_WindowContexts.pop_back();
				}
				else
				{
					++currentIndex;
				}
			}
		}
		glfwPollEvents();
	}

	void CVulkanApplication::ReleaseAllWindowContexts()
	{
		m_WindowContexts.clear();
	}


	void CVulkanApplication::ExecuteSubpass_SimpleDraw(
		CRenderpassBuilder const& inRenderPass
		, uint32_t subpassID
		, uint32_t width
		, uint32_t height
		, vk::CommandBuffer cmd)
	{
		auto& renderPassInfo = inRenderPass.GetRenderPassInfo();
		RenderPassDescriptor rpDesc{ renderPassInfo };
		auto pRenderPass = m_GPUObjectManager.GetRenderPassCache().GetOrCreate(rpDesc).lock();

		auto& subpassData = inRenderPass.GetSubpassData_SimpleDrawcall(subpassID);
		auto vertModule = m_GPUObjectManager.GetShaderModuleCache().GetOrCreate({subpassData.shaderSet.vert}).lock();
		auto fragModule = m_GPUObjectManager.GetShaderModuleCache().GetOrCreate({subpassData.shaderSet.frag}).lock();

		CPipelineObjectDescriptor pipelineDesc{
			subpassData.pipelineStateObject
			, subpassData.vertexInputDescriptor
			, ShaderStateDescriptor{vertModule, fragModule}
			, pRenderPass
			, subpassID };

		auto pPipeline = m_GPUObjectManager.GetPipelineCache().GetOrCreate(pipelineDesc).lock();

	

		cmd.setViewport(0
			, {
				vk::Viewport{0.0f, 0.0f
				, static_cast<float>(width)
				, static_cast<float>(height)
				, 0.0f, 1.0f}
			}
		);
		cmd.setScissor(0
			, {
				vk::Rect2D{
					{0, 0}
					, { width, height }
				}
			}
		);
		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pPipeline->GetPipeline());
		CCommandList_Impl cmdListInterface{ cmd, pRenderPass, subpassID };
		subpassData.commandFunction(cmdListInterface);
	}

	void CVulkanApplication::ExecuteRenderPass(CRenderpassBuilder const& inRenderPass)
	{
		auto newTask = NewTask();
		newTask->Functor([this, inRenderPass]()
			{
				auto& renderPassInfo = inRenderPass.GetRenderPassInfo();
				auto& threadContext = AquireThreadContext();
				RenderPassDescriptor rpDesc{ renderPassInfo };
				auto pRenderPass = m_GPUObjectManager.GetRenderPassCache().GetOrCreate(rpDesc).lock();
				auto textureSize = m_WindowContexts[0]->GetSize();
				FramebufferDescriptor fbDesc{ {m_WindowContexts[0]->GetCurrentFrameImageView()}
				,  pRenderPass
				, textureSize.x, textureSize.y, 1 };
				auto pFrameBufferObject = m_GPUObjectManager.GetFramebufferCache().GetOrCreate(fbDesc).lock();
				std::vector<vk::ClearValue> clearValues;
				clearValues.resize(renderPassInfo.attachmentInfos.size());
				for (uint32_t attachmentID = 0; attachmentID < renderPassInfo.attachmentInfos.size(); ++attachmentID)
				{
					auto& attachmentInfo = renderPassInfo.attachmentInfos[attachmentID];
					clearValues[attachmentID] = AttachmentClearValueTranslate(
						attachmentInfo.clearValue
						, attachmentInfo.format);
				}

				auto cmd = threadContext.GetCurrentFramePool().AllocateOnetimeCommandBuffer();

				VulkanBarrierCollector barrier{ GetSubmitCounterContext().GetGraphicsQueueFamily() };
				barrier.PushImageBarrier(m_WindowContexts[0]->GetCurrentFrameImage()
					, ResourceUsage::eDontCare, ResourceUsage::eColorAttachmentOutput);
				barrier.ExecuteBarrier(cmd);

				cmd.beginRenderPass(
					vk::RenderPassBeginInfo{
					pRenderPass->GetRenderPass()
						, pFrameBufferObject->GetFramebuffer()
						, vk::Rect2D{{0, 0}
					, { pFrameBufferObject->GetWidth()
						, pFrameBufferObject->GetHeight() }}
					, clearValues}
				, vk::SubpassContents::eInline);

				for (uint32_t subpassId = 0; subpassId < renderPassInfo.subpassInfos.size(); ++subpassId)
				{
					ESubpassType subpasType = inRenderPass.GetSubpassType(subpassId);
					if (subpassId > 0)
					{
						cmd.nextSubpass(vk::SubpassContents::eInline);
					}
					switch (subpasType)
					{
					case ESubpassType::eSimpleDraw:
						ExecuteSubpass_SimpleDraw(
							inRenderPass
							, subpassId
							, pFrameBufferObject->GetWidth()
							, pFrameBufferObject->GetHeight()
							, cmd);
						break;
					case ESubpassType::eMultiDrawInterface:
						break;
					}
				}
				cmd.endRenderPass();
				cmd.end();
				ReturnThreadContext(threadContext);
			});
	}

	CVulkanApplication::CVulkanApplication() :
	m_GPUBufferPool(*this)
	, m_GPUObjectManager(*this)
	, m_MemoryManager(*this)
	, m_RenderGraphDic(*this)
	{
	}

	CVulkanApplication::~CVulkanApplication()
	{
		//ReleaseApp();
	}

	void CVulkanApplication::InitApp(std::string const& appName, std::string const& engineName)
	{
		InitializeInstance(appName, engineName);
		EnumeratePhysicalDevices();
		CreateDevice();
		m_MemoryManager.Initialize();
	}

	void CVulkanApplication::ReleaseApp()
	{
		DeviceWaitIdle();
		m_GPUBufferPool.ReleaseAll();
		m_MemoryManager.Release();
		DestroyThreadContexts();
		ReleaseAllWindowContexts();
		DestroyDevice();
		m_PhysicalDevice = nullptr;
		DestroyInstance();
	}

	void CVulkanApplication::DeviceWaitIdle()
	{
		m_TaskFuture.wait();
		if (m_Device != vk::Device(nullptr))
		{
			m_Device.waitIdle();
		}
	}

}

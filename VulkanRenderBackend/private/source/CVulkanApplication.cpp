#include "private/include/pch.h"
#include "private/include/Utils.h"
#include <private/include/RenderBackendSettings.h>
#include "private/include/CVulkanApplication.h"
#include "private/include/CVulkanThreadContext.h"
#include <private/include/CVulkanBufferObject.h>

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
					uint32_t const releasedFrame = m_SubmitCounterContext.GetReleasedFrameID();
					for (auto itrThreadContext = m_ThreadContexts.begin(); itrThreadContext != m_ThreadContexts.end(); ++itrThreadContext)
					{
						itrThreadContext->DoReleaseResourceBeforeFrame(releasedFrame);
					}
					m_MemoryManager.ReleaseCurrentFrameResource();
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
					vk::ResultValue<uint32_t> currentBuffer = GetDevice().acquireNextImageKHR(
						m_WindowContexts[0].m_Swapchain
						, 100000
						, m_WindowContexts[0].m_WaitNextFrameSemaphore, nullptr);

					std::array<const vk::Semaphore, 1> semaphore = { m_WindowContexts[0].m_WaitNextFrameSemaphore };
					std::array<const vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eTransfer };
					std::array<const vk::Semaphore, 1> presentSemaphore = { m_WindowContexts[0].m_CanPresentSemaphore };

					auto& threadContext0 = m_ThreadContexts[0];
					auto cmd = threadContext0.GetCurrentFramePool().AllocateOnetimeCommandBuffer();

					cmd.pipelineBarrier(
						vk::PipelineStageFlagBits::eTopOfPipe
						, vk::PipelineStageFlagBits::eTransfer
						, {}
						, {}
						, {}
						, vk::ImageMemoryBarrier(
							vk::AccessFlagBits::eNone
							, vk::AccessFlagBits::eTransferWrite
							, vk::ImageLayout::eUndefined
							, vk::ImageLayout::eTransferDstOptimal
							, {}, {}
							, m_WindowContexts[0].m_SwapchainImages[currentBuffer.value]
							, vulkan_backend::utils::DefaultColorSubresourceRange()
						));

					cmd.clearColorImage(
						m_WindowContexts[0].m_SwapchainImages[currentBuffer.value]
						, vk::ImageLayout::eTransferDstOptimal
						, vk::ClearColorValue(std::array<float, 4>{ 0.0f, 0.0f, 1.0f, 1.0f }), vulkan_backend::utils::DefaultColorSubresourceRange());

					cmd.pipelineBarrier(
						vk::PipelineStageFlagBits::eTransfer
						, vk::PipelineStageFlagBits::eBottomOfPipe
						, {}
						, {}
						, {}
						, vk::ImageMemoryBarrier(
							vk::AccessFlagBits::eTransferWrite
							, vk::AccessFlagBits::eNone
							, vk::ImageLayout::eTransferDstOptimal
							, vk::ImageLayout::ePresentSrcKHR
							, {}, {}
							, m_WindowContexts[0].m_SwapchainImages[currentBuffer.value]
							, vulkan_backend::utils::DefaultColorSubresourceRange()
						));

					cmd.end();
					waitingSubmitCommands.push_back(cmd);

					m_SubmitCounterContext.FinalizeCurrentFrameGraphics(waitingSubmitCommands
						, semaphore
						, waitStages
						, presentSemaphore);

					vk::PresentInfoKHR presenttInfo(
						presentSemaphore
						, m_WindowContexts[0].m_Swapchain
						, currentBuffer.value
					);
					m_WindowContexts[0].m_PresentQueue.second.presentKHR(presenttInfo);
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

	void CVulkanApplication::RunGraphWithPresentTarget(std::string const& targetName)
	{
		CTaskGraph* newGraph = GetThreadManager()->NewTaskGraph()->Name("SubGraph");

		newGraph->FinalizeFunctor([this, targetName]()
			{

				std::cout << targetName << std::endl;
			});
		CTask* subGraphTask = NewTask()
			->Name("SubGraph Task")
			->Functor([this, newGraph]()
				{
					GetThreadManager()->ExecuteTaskGraph(newGraph);
				});
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

	CGPUPrimitiveResource_Vulkan* CVulkanApplication::NewPrimitiveResource()
	{
		return m_PrimitiveResourcePool.Alloc(this);
	}

	void CVulkanApplication::DestroyPrimitiveResource(CGPUPrimitiveResource_Vulkan* resource)
	{
		m_PrimitiveResourcePool.Release(resource);
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

		assert(!generalUsageQueues.empty());

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
		assert(threadCount > 0);
		assert(m_ThreadContexts.size() == 0);
		p_ThreadManager = threadManager;
		m_ThreadContexts.reserve(threadCount);
		for (uint32_t threadContextId = 0; threadContextId < threadCount; ++threadContextId)
		{
			//SubObject_EmplaceBack(m_ThreadContexts, threadContextId);
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

	CVulkanMemoryManager& CVulkanApplication::GetMemoryManager() const
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

	//CTaskGraph* CVulkanApplication::GetCurrentFrameTaskGraph() const
	//{
	//	return p_TaskGraph;
	//}

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

	void CVulkanApplication::CreateWindowContext(std::string windowName, uint32_t initialWidth, uint32_t initialHeight)
	{
		m_WindowContexts.emplace_back(windowName, initialWidth, initialHeight);
		auto& newContext = m_WindowContexts.back();
		newContext.Initialize(this);
	}

	void CVulkanApplication::TickWindowContexts()
	{
		bool anyNeedClose = std::any_of(m_WindowContexts.begin(), m_WindowContexts.end(), [](CWindowContext const& wcontest)
			{
				return wcontest.NeedClose();
			});
		if(anyNeedClose)
		{
			size_t lastIndex = m_WindowContexts.size();
			size_t currentIndex = 0;
			while (currentIndex < m_WindowContexts.size())
			{
				if (m_WindowContexts[currentIndex].NeedClose())
				{
					m_WindowContexts[currentIndex].Release();
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

	CVulkanApplication::CVulkanApplication() :
	m_PrimitiveResourcePool()
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
		m_MemoryManager.Initialize(this);
	}

	void CVulkanApplication::ReleaseApp()
	{
		DeviceWaitIdle();
		m_MemoryManager.Release();
		DestroyThreadContexts();
		ReleaseAllWindowContexts();
		DestroyDevice();
		m_PhysicalDevice = nullptr;
		DestroyInstance();
	}

	void CVulkanApplication::DeviceWaitIdle()
	{
		if (m_Device != vk::Device(nullptr))
		{
			m_Device.waitIdle();
		}
	}

}

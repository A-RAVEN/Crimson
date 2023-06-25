#include "private/include/pch.h"
#include "private/include/Utils.h"
#include <private/include/RenderBackendSettings.h>
#include "private/include/CVulkanApplication.h"
#include "private/include/CVulkanThreadContext.h"
#include <private/include/CVulkanBufferObject.h>

namespace graphics_backend
{
	void CVulkanApplication::TickRunTest()
	{
		m_SubmitCounterContext.WaitingForCurrentFrame();
		if (m_SubmitCounterContext.GetCurrentFrameID() == 0)
		{
			CVulkanThreadContext* threadContext = GetThreadContext(0);
			auto bufferObject = threadContext->AllocBufferObject(true, 4 * 4
				, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer);
			auto bufferObject1 = threadContext->AllocBufferObject(false, 4 * 4
				, vk::BufferUsageFlagBits::eTransferSrc);
			auto& currentFramePool = threadContext->GetCurrentFramePool();
			vk::CommandBuffer cmd = currentFramePool.AllocateCommandBuffer();
			cmd.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
			cmd.copyBuffer(bufferObject1->GetBuffer(), bufferObject->GetBuffer(), vk::BufferCopy(0, 0, bufferObject1->GetAllocationInfo().size));
			cmd.end();
			std::vector<vk::CommandBuffer> commands;
			threadContext->CollectSubmittingCommandBuffers(commands);
			m_SubmitCounterContext.SubmitCurrentFrameGraphics(commands);
		}

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

		m_Instance = vk::createInstance(instance_info);

		vulkan_backend::utils::SetupVulkanInstanceFunctionPointers(m_Instance);


	#if !defined(NDEBUG)
		m_DebugMessager = m_Instance.createDebugUtilsMessengerEXT(vulkan_backend::utils::makeDebugUtilsMessengerCreateInfoEXT());
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

		auto extensions = GetDeviceExtensionNames();
		vk::DeviceCreateInfo deviceCreateInfo({}, deviceQueueCreateInfoList, {}, extensions);

		m_Device = m_PhysicalDevice.createDevice(deviceCreateInfo);
		vk::Queue generalQueue = m_Device.getQueue(generalQueueRef.first, generalQueueRef.second);
		vk::Queue computeQueue = m_Device.getQueue(computeQueueRef.first, computeQueueRef.second);
		vk::Queue transferQueue = m_Device.getQueue(transferQueueRef.first, transferQueueRef.second);
	
		m_SubmitCounterContext.Initialize(this);
		m_SubmitCounterContext.InitializeSubmitQueues(generalQueue, computeQueue, transferQueue);
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

	void CVulkanApplication::InitializeThreadContext(uint32_t threadCount)
	{
		assert(threadCount > 0);
		assert(m_ThreadContexts.size() == 0);
		m_ThreadContexts.reserve(threadCount);
		for (uint32_t threadContextId = 0; threadContextId < threadCount; ++threadContextId)
		{
			//SubObject_EmplaceBack(m_ThreadContexts, threadContextId);
			m_ThreadContexts.push_back(SubObject<CVulkanThreadContext>(threadContextId));
		}
	}

	void CVulkanApplication::DestroyThreadContexts()
	{
		for (auto& threadContext : m_ThreadContexts)
		{
			ReleaseSubObject(threadContext);
		}
		m_ThreadContexts.clear();
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

	CVulkanApplication::~CVulkanApplication()
	{
		ReleaseApp();
	}

	void CVulkanApplication::InitApp(std::string const& appName, std::string const& engineName)
	{
		InitializeInstance(appName, engineName);
		EnumeratePhysicalDevices();
		CreateDevice();
	}

	void CVulkanApplication::ReleaseApp()
	{
		DeviceWaitIdle();
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

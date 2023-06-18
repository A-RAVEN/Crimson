#include "private/include/pch.h"
#include "private/include/Utils.h"
#include <private/include/RenderBackendSettings.h>
#include "private/include/CVulkanApplication.h"
#include "private/include/CVulkanThreadContext.h"

namespace graphics_backend
{

	void CVulkanApplication::InitializeInstance(std::string const& name, std::string const& engine_name)
	{
		vk::ApplicationInfo application_info(
			name.c_str()
			, 1
			, engine_name.c_str()
			, VK_API_VERSION_1_3);

		std::array<const char*, 1> extensionNames = {
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME
		};

		auto extensions = GetInstanceExtensionNames();
		vk::InstanceCreateInfo instance_info({}, &application_info, {}, extensions);

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

		auto queueFamilyPropIterator = std::find_if(queueFamilyProperties.begin()
			, queueFamilyProperties.end()
			, [](vk::QueueFamilyProperties const& itrProperties) {
				return itrProperties.queueFlags & vk::QueueFlagBits::eGraphics;
			});

		size_t queueFamilyIndex = std::distance(queueFamilyProperties.begin(), queueFamilyPropIterator);

		assert(queueFamilyIndex < queueFamilyProperties.size());

		std::vector<float> queuePrioritiese = { 0.0f };
		vk::DeviceQueueCreateInfo queueCreateInfo(vk::DeviceQueueCreateFlags()
			, static_cast<uint32_t>(queueFamilyIndex)
			, queuePrioritiese);

		auto extensions = GetDeviceExtensionNames();
		vk::DeviceCreateInfo deviceCreateInfo({}, queueCreateInfo, {}, extensions);

		m_Device = m_PhysicalDevice.createDevice(deviceCreateInfo);
	}

	void CVulkanApplication::DestroyDevice()
	{
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
			m_ThreadContexts.emplace_back(this);
		}
	}

	void CVulkanApplication::DestroyThreadContexts()
	{
		m_ThreadContexts.clear();
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
		DestroyDevice();
		m_PhysicalDevice = nullptr;
		DestroyInstance();
	}

}

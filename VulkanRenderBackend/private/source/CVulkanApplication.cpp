#include "private/include/pch.h"
#include "private/include/CVulkanApplication.h"
#include "private/include/Utils.h"

void CVulkanApplication::InitializeInstance(std::string const& name, std::string const& engine_name)
{
	vk::ApplicationInfo application_info(
		name.c_str()
		, 1
		, engine_name.c_str()
		, VK_API_VERSION_1_3);

	vk::InstanceCreateInfo instance_info({}, &application_info);

	m_Instance = vk::createInstance(instance_info);

#if !defined(NDEBUG)
	//m_DebugMessager = m_Instance.createDebugUtilsMessengerEXT(vulkan_backend::utils::makeDebugUtilsMessengerCreateInfoEXT());
#endif
}

void CVulkanApplication::DestroyInstance()
{
	if (m_Instance != vk::Instance(nullptr))
	{
#if !defined( NDEBUG )
		//m_Instance.destroyDebugUtilsMessengerEXT(m_DebugMessager);
		//m_DebugMessager = nullptr;
#endif
		m_Instance.destroy();
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
	vk::DeviceCreateInfo deviceCreateInfo(vk::DeviceCreateFlags(), queueCreateInfo);

	m_Device = m_PhysicalDevice.createDevice(deviceCreateInfo);
}

void CVulkanApplication::DestroyDevice()
{
	m_Device.destroy();
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


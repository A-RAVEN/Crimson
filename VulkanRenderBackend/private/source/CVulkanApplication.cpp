#include "private/include/pch.h"
#include "private/include/CVulkanApplication.h"

void CVulkanApplication::InitializeInstance(std::string const& name, std::string const& engine_name)
{
	vk::ApplicationInfo application_info(
		name.c_str()
		, 1
		, engine_name.c_str()
		, VK_API_VERSION_1_3);

	vk::InstanceCreateInfo instance_info({}, &application_info);

	m_Instance = vk::createInstance(instance_info);
}

void CVulkanApplication::DestroyInstance()
{
	if (m_Instance != vk::Instance(nullptr))
	{
		m_Instance.destroy();
	}
}

void CVulkanApplication::EnumerateDevices()
{
	m_PhysicalDevice = m_Instance.enumeratePhysicalDevices().front();
}

CVulkanApplication::~CVulkanApplication()
{
	m_PhysicalDevice = nullptr;
	DestroyInstance();
}

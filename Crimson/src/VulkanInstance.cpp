#include <headers/VulkanInstance.h>
#include <include/Version.h>
#include <headers/VulkanDebugLog.h>
#include <vector>
#include <array>
#include <map>

namespace Crimson
{
	VulkanInstance* VulkanInstance::p_Singleton = nullptr;

	void VulkanInstance::Init(bool enable_debug_extension)
	{
		if (p_Singleton == nullptr) 
		{
			p_Singleton = new VulkanInstance(enable_debug_extension);
		}
	}

	void VulkanInstance::Dispose()
	{
		if (p_Singleton != nullptr)
		{
			delete p_Singleton;
			p_Singleton = nullptr;
		}
	}

	VulkanInstance* VulkanInstance::Get()
	{
		return p_Singleton;
	}

	VkInstance VulkanInstance::GetVulkanInstance()
	{
		return m_VulkanInstance;
	}

	std::vector<VkPhysicalDevice> const& VulkanInstance::GetPhysicalDevices()
	{
		return m_PhysicalDevices;
	}

	VulkanInstance::VulkanInstance(bool enable_debug_extension) : m_DebugReportCallBack(VK_NULL_HANDLE)
	{
		std::vector<char const*> instance_extension_names = GetFilteredInstanceExtensions(
		{
			VK_KHR_SURFACE_EXTENSION_NAME,
			CURRENT_VULKAN_SURFACE_EXTENSION_NAME,

			VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
		});

		if (enable_debug_extension)
		{
			instance_extension_names.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}

		std::array<char const*, 1> instance_validation_names =
		{
			"VK_LAYER_LUNARG_standard_validation"
		};

		//Create application info
		VkApplicationInfo app_info{};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pNext = nullptr;
		app_info.apiVersion = CURRENT_VULKAN_VERSION;
		app_info.pApplicationName = APPLICATION_NAME;
		app_info.applicationVersion = APPLICATION_VERSION;
		app_info.pEngineName = APPLICATION_NAME;
		app_info.engineVersion = APPLICATION_VERSION;

		//Create Vulkan application instance creation info
		VkInstanceCreateInfo instance_create_info{};
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pNext = nullptr;
		instance_create_info.flags = VK_NULL_HANDLE;
		instance_create_info.pApplicationInfo = &app_info;
		instance_create_info.enabledLayerCount = static_cast<uint32_t>(instance_validation_names.size());
		instance_create_info.ppEnabledLayerNames = instance_validation_names.data();
		instance_create_info.enabledExtensionCount = static_cast<uint32_t>(instance_extension_names.size());
		instance_create_info.ppEnabledExtensionNames = instance_extension_names.data();

		//Create Vulkan application instance
		CHECK_VKRESULT(vkCreateInstance(&instance_create_info, VULKAN_ALLOCATOR_POINTER, &m_VulkanInstance), "Vulkan Instance Creation Failed!");
		if (enable_debug_extension) 
		{ 
			SetupDebugCallback(); 
		}

		//Enumerate Physical Devices
		EnumeratePhysicalDevices();
	}
	VulkanInstance::~VulkanInstance()
	{
		ClearPhysicalDevices();
		DestroyDebugCallback();
		vkDestroyInstance(m_VulkanInstance, VULKAN_ALLOCATOR_POINTER);
	}
	std::vector<char const*> VulkanInstance::GetFilteredInstanceExtensions(std::vector<char const*> const& wanted_extensions)
	{
		uint32_t ext_count = 0;
		CHECK_VKRESULT(vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr), "Vulkan Enumerate Instance Extensions Issue!");
		std::vector<VkExtensionProperties> properties(ext_count);
		CHECK_VKRESULT(vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, properties.data()), "Vulkan Enumerate Instance Extensions Issue!");
		std::vector<char const*> return_val;
		for (auto name : wanted_extensions)
		{
			for (auto& itr_property : properties)
			{
				if (std::string(name) == std::string(itr_property.extensionName))
				{
					return_val.push_back(name);
					break;
				}
			}
		}
		return return_val;
	}
	uint32_t VulkanInstance::EvaluateVulkanPhysicalDevice(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device, &properties);
		uint32_t score = 0;
		switch (properties.deviceType)
		{
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			score += 10;
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			score += 1;
		}
		return score;
	}
	void VulkanInstance::SetupDebugCallback()
	{
		uint32_t layer_num;
		std::vector<VkLayerProperties> available_layers;
		vkEnumerateInstanceLayerProperties(&layer_num, nullptr);
		available_layers.resize(layer_num);
		vkEnumerateInstanceLayerProperties(&layer_num, available_layers.data());

		uint32_t extension_num = 0;
		std::vector<VkExtensionProperties> extensions;
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_num, nullptr);
		extensions.resize(extension_num);
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_num, extensions.data());

		VkDebugReportCallbackCreateInfoEXT callback_createInfo = {};
		callback_createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		callback_createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
		callback_createInfo.pfnCallback = VulkanDebug::VulkanDebugCallback;

		auto p_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_VulkanInstance, "vkCreateDebugReportCallbackEXT");
		if (p_vkCreateDebugReportCallbackEXT != nullptr)
		{
			CHECK_VKRESULT(p_vkCreateDebugReportCallbackEXT(m_VulkanInstance, &callback_createInfo, VULKAN_ALLOCATOR_POINTER, &m_DebugReportCallBack), "Vulkan Debug Callback Creation Failed!");
		}
		else
		{
			//TODO: Report Error
		}
	}
	void VulkanInstance::DestroyDebugCallback()
	{
		if (m_DebugReportCallBack != VK_NULL_HANDLE)
		{
			auto p_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_VulkanInstance, "vkDestroyDebugReportCallbackEXT");
			if (p_vkDestroyDebugReportCallbackEXT != nullptr) {
				p_vkDestroyDebugReportCallbackEXT(m_VulkanInstance, m_DebugReportCallBack, VULKAN_ALLOCATOR_POINTER);
			}
		}
	}
	void VulkanInstance::EnumeratePhysicalDevices()
	{
		//check & get physical devices' infos
		uint32_t count(0);
		CHECK_VKRESULT(vkEnumeratePhysicalDevices(m_VulkanInstance, &count, VK_NULL_HANDLE), "Enumerate Vulkan Physical Devices Issue!");
		m_PhysicalDevices.resize(count);

		CHECK_VKRESULT(vkEnumeratePhysicalDevices(m_VulkanInstance, &count, m_PhysicalDevices.data()), "Get Physical Devices Irror!");
		
		std::map<uint32_t, VkPhysicalDevice> device_sort;
		for (uint32_t i = 0; i < count; ++i)
		{
			device_sort.insert(std::make_pair(EvaluateVulkanPhysicalDevice(m_PhysicalDevices[i]), m_PhysicalDevices[i]));
		}
		m_PhysicalDevices.clear();
		for (auto itr = device_sort.rbegin(); itr != device_sort.rend(); ++itr)
		{
			m_PhysicalDevices.push_back(itr->second);
		}
	}
	void VulkanInstance::ClearPhysicalDevices()
	{
		m_PhysicalDevices.clear();
	}
}
#pragma once
#include "VulkanHeader.h"
#include <vector>

namespace Crimson
{

	class VulkanInstance
	{
	public:
		static void Init(bool enable_debug_extension);
		static void Dispose();
		static VulkanInstance* Get();
		VkInstance GetVulkanInstance();
		std::vector<VkPhysicalDevice> const& GetPhysicalDevices();
		uint32_t EvaluateVulkanPhysicalDevice(VkPhysicalDevice device);
	private:
		VulkanInstance(bool enable_debug_extension);
		~VulkanInstance();
		//Debug Callbacks
		void SetupDebugCallback();
		void DestroyDebugCallback();
		//Physical Devices
		void EnumeratePhysicalDevices();
		void ClearPhysicalDevices();
		VkInstance m_VulkanInstance;
		std::vector<VkPhysicalDevice> m_PhysicalDevices;

		VkDebugReportCallbackEXT m_DebugReportCallBack;


		static VulkanInstance* p_Singleton;
	};
}
#pragma once
namespace vulkan_backend
{
	namespace utils
	{
		vk::DebugUtilsMessengerCreateInfoEXT makeDebugUtilsMessengerCreateInfoEXT();

		void SetupVulkanInstanceFunctionPointers(vk::Instance const& inInstance);
		void SetupVulkanDeviceFunctinoPointers(vk::Device const& inDevice);
		void CleanupVulkanInstanceFuncitonPointers();
	}
}
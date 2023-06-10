#pragma once
class CVulkanApplication
{
public:
	~CVulkanApplication();
private:
	void InitializeInstance(std::string const& name, std::string const& engine_name);
	void DestroyInstance();
	void EnumeratePhysicalDevices();
	void CreateDevice();
private:
	vk::Instance m_Instance = nullptr;
	vk::PhysicalDevice m_PhysicalDevice = nullptr;
	vk::Device m_Device = nullptr;
#if !defined(NDEBUG)
	vk::DebugUtilsMessengerEXT m_DebugMessager = nullptr;
#endif
};


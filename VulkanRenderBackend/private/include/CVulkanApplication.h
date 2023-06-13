#pragma once
class CVulkanApplication
{
public:
	~CVulkanApplication();
	void InitApp(std::string const& appName, std::string const& engineName);
	void ReleaseApp();
private:
	void InitializeInstance(std::string const& name, std::string const& engineName);
	void DestroyInstance();
	void EnumeratePhysicalDevices();
	void CreateDevice();
	void DestroyDevice();
private:
	vk::Instance m_Instance = nullptr;
	vk::PhysicalDevice m_PhysicalDevice = nullptr;
	vk::Device m_Device = nullptr;
#if !defined(NDEBUG)
	vk::DebugUtilsMessengerEXT m_DebugMessager = nullptr;
#endif
};

extern "C"
{

}
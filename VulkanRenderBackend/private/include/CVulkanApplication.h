#pragma once
class CVulkanApplication
{
public:
	~CVulkanApplication();
private:
	void InitializeInstance(std::string const& name, std::string const& engine_name);
	void DestroyInstance();
	void EnumerateDevices();
private:
	vk::Instance m_Instance = nullptr;
	vk::PhysicalDevice m_PhysicalDevice = nullptr;
};


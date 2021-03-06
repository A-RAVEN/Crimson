#include <include/GPUDevice.h>
#include <headers/VulkanInstance.h>
#include <headers/VulkanGPUDevice.h>

namespace Crimson
{
	GPUDeviceManager* GPUDeviceManager::p_Singleton = nullptr;

	GPUDeviceManager::GPUDeviceManager(){}

	GPUDeviceManager::~GPUDeviceManager()
	{

	}

	PGPUDevice GPUDeviceManager::CreateVulkanDevice(uint32_t physics_device_id, uint32_t prefered_graphics_queue_num, uint32_t prefered_compute_queue_num, uint32_t prefered_transfer_queue_num)
	{
		VulkanGPUDevice* new_device = new VulkanGPUDevice();
		new_device->InitVulkanDevice(physics_device_id, prefered_graphics_queue_num, prefered_compute_queue_num, prefered_transfer_queue_num);
		return new_device;
	}

	void GPUDeviceManager::Init()
	{
		if (p_Singleton == nullptr)
		{
			p_Singleton = new GPUDeviceManager();
		}
	}
	void GPUDeviceManager::InitAPIContext(EAPIType type, bool enable_debug_system)
	{
		switch (type)
		{
		case EAPIType::E_API_TYPE_VULKAN:
#ifndef CRIMSON_NO_VULKAN
			VulkanInstance::Init(enable_debug_system);
#endif
			break;
		case EAPIType::E_API_TYPE_D3D12:
#ifndef CRIMSON_NO_D3D12
			//init d3d12 instance
#endif
			break;
		}
	}
	GPUDeviceManager* GPUDeviceManager::Get()
	{
		return p_Singleton;
	}
	void GPUDeviceManager::Dispose()
	{
		if (p_Singleton != nullptr)
		{
			delete p_Singleton;
			p_Singleton = nullptr;
		}
	}
	PGPUDevice GPUDeviceManager::CreateDevice(std::string const& name, uint32_t physics_device_id, EAPIType type, uint32_t prefered_graphics_queue_num, uint32_t prefered_compute_queue_num, uint32_t prefered_transfer_queue_num)
	{
		PGPUDevice return_device = nullptr;
		switch (type)
		{
			case EAPIType::E_API_TYPE_VULKAN:
#ifndef CRIMSON_NO_VULKAN
				return_device = CreateVulkanDevice(physics_device_id, prefered_graphics_queue_num, prefered_compute_queue_num, prefered_transfer_queue_num);
				return_device->m_Name = name;
#endif
				break;
			case EAPIType::E_API_TYPE_D3D12:
#ifndef CRIMSON_NO_D3D12
#endif
				break;
		}
		if (return_device == nullptr)
		{
			//Debug Log
		}
		else
		{
			m_DeviceList.push_back(return_device);
			m_DeviceMap.insert(std::make_pair(name, m_DeviceList.size() - 1));
		}
		return return_device;
	}
	PGPUDevice GPUDeviceManager::GetDevice(std::string const& name)
	{
		auto find = m_DeviceMap.find(name);
		if (find != m_DeviceMap.end())
		{
			return m_DeviceList[find->second];
		}
		return nullptr;
	}
	PGPUDevice GPUDeviceManager::GetDevice(uint32_t id)
	{
		if (id < m_DeviceList.size())
		{
			return m_DeviceList[id];
		}
		return nullptr;
	}
}
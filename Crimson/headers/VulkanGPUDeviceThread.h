#pragma once
#include <include/GPUDevice.h>

namespace Crimson
{
	class VulkanGPUDevice;
	class VulkanGPUDeviceThread : public IGPUDeviceThread
	{
	public:
		friend class VulkanGPUDevice;
	private:
		void InitGPUDeviceThread(VulkanGPUDevice* device);
		VulkanGPUDevice* p_GPUDevice;
	};
}
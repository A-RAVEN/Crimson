#include<headers/VulkanImage.h>

namespace Crimson
{
	VulkanImageObject::VulkanImageObject() : IGPUImage(),
		p_OwningDevice(nullptr),
		m_Image(VK_NULL_HANDLE),
		m_Allocation(nullptr)
	{}
	void VulkanImageObject::SetVulkanImage(VulkanGPUDevice* p_device, VkImage image, VmaAllocation allocation_info, EFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mip_level_num, uint32_t layer_num, std::vector<EImageUsage> const& usages, EMemoryType memory_type)
	{
		p_OwningDevice = p_device;
		m_Image = image;
		m_Allocation = allocation_info;
		m_Format = format;
		m_Width = width;
		m_Height = height;
		m_Depth = depth;
		m_MipLevelNum = mip_level_num;
		m_LayerNum = layer_num;
		m_ImageUsages = usages;
		m_MemoryType = memory_type;
	}
	void VulkanImageObject::Dispose()
	{
		p_OwningDevice->HandleDisposedImage(this);
	}
}
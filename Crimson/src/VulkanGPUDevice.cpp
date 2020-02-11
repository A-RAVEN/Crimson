#include <headers/VulkanGPUDevice.h>
#include <headers/VulkanInstance.h>
#include <headers/VulkanDebugLog.h>
#include <headers/VulkanTranslator.h>
#include <headers/VulkanBuffer.h>
#include <headers/VulkanImage.h>
#include <headers/VulkanRenderPass.h>
#include <headers/VulkanDescriptors.h>
#include <headers/VulkanPipeline.h>
#include <headers/VulkanRayTracer.h>
#include <headers/VulkanFramebuffer.h>
#include <headers/VulkanRenderPassInstance.h>
#include <headers/VulkanGPUDeviceThread.h>
#include <headers/VulkanRayTraceGeometry.h>
#include <headers/VulkanAccelerationStructure.h>
#include <headers/VulkanBatch.h>
#include <headers/vk_mem_alloc.h>
#include <algorithm>
#include <limits>
#include <deque>

namespace Crimson
{
	void VulkanGPUDevice::InitDeviceChannel(uint32_t num_channel)
	{
	}
	void VulkanGPUDevice::RegisterWindow(IWindow& window)
	{
		auto find = m_SurfaceContexts.find(window.GetName());
		if (find == m_SurfaceContexts.end())
		{
			VkSurfaceKHR window_surface = VK_NULL_HANDLE;
#ifdef _WIN32
			VkWin32SurfaceCreateInfoKHR surface_create_info = {};
			surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surface_create_info.pNext = nullptr;
			surface_create_info.hinstance = window.GetWin32Instance();
			surface_create_info.hwnd = window.GetWin32Handle();
			CHECK_VKRESULT(vkCreateWin32SurfaceKHR(VulkanInstance::Get()->GetVulkanInstance(), &surface_create_info, VULKAN_ALLOCATOR_POINTER, &window_surface), "Vulkan Win32 Surface Creation Issue!");
#endif
			m_SurfaceContexts.insert(std::make_pair(window.GetName(), VulkanSurfaceContext()));
			m_SurfaceContexts[window.GetName()].InitSurfaceContext(this, window_surface);
		}
	}
	PGPUDeviceThread VulkanGPUDevice::CreateThread()
	{
		VulkanGPUDeviceThread* new_thread = new VulkanGPUDeviceThread();
		new_thread->InitGPUDeviceThread(this);
		m_Threads.insert(new_thread);
		return new_thread;
	}
	void VulkanGPUDevice::HandleDisposedThread(VulkanGPUDeviceThread* p_thread)
	{
		m_Threads.erase(p_thread);
		delete p_thread;
	}
	PGPUBuffer VulkanGPUDevice::CreateBuffer(uint64_t buffer_size, std::vector<EBufferUsage> const& usages, EMemoryType memory_type)
	{
		VkBufferCreateInfo buffer_create_info{};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = buffer_size;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		buffer_create_info.usage = TranslateBufferUsageFlagsToVulkan(usages);


		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = TranslateMemoryUsageToVMA(memory_type);
		
		VkBuffer new_buffer = VK_NULL_HANDLE;
		VmaAllocation new_allocation = nullptr;
		CHECK_VKRESULT(vmaCreateBuffer(m_MemoryAllocator, &buffer_create_info, &allocInfo, &new_buffer, &new_allocation, nullptr), "VMA Creating Buffer Issue!");
		VulkanBufferObject* new_buffer_object = new VulkanBufferObject();
		new_buffer_object->SetVulkanBuffer(this, new_buffer, new_allocation, buffer_size, usages, memory_type);
		return new_buffer_object;
	}
	void VulkanGPUDevice::HandleDisposedBuffer(VulkanBufferObject* p_buffer)
	{
		vmaDestroyBuffer(m_MemoryAllocator, p_buffer->m_Buffer, p_buffer->m_Allocation);
		delete p_buffer;
	}
	PGPUImage VulkanGPUDevice::CreateImage(EFormat format, uint32_t width, uint32_t height, uint32_t depth, std::vector<EImageUsage> const& usages, EMemoryType memory_type, uint32_t layer_num, uint32_t mip_level_num, uint32_t sample_num)
	{
		VkImageCreateInfo image_create_info{};
		image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.format = TranslateImageFormatToVulkan(format);
		image_create_info.flags = 0;	//reserved
		image_create_info.imageType = depth == 1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_3D;
		image_create_info.extent.width = width;
		image_create_info.extent.height = height;
		image_create_info.extent.depth = depth;
		image_create_info.mipLevels = mip_level_num;
		image_create_info.arrayLayers = layer_num;
		image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image_create_info.usage = TranslateImageUsageFlagsToVulkan(usages);

		VmaAllocationCreateInfo alloc_info = {};
		alloc_info.usage = TranslateMemoryUsageToVMA(memory_type);

		VkImage new_image = VK_NULL_HANDLE;
		VmaAllocation new_allocation = nullptr;
		CHECK_VKRESULT(vmaCreateImage(m_MemoryAllocator, &image_create_info, &alloc_info, &new_image, &new_allocation, nullptr), "Vulkan Create Image Issue!");
		
		VulkanImageObject* new_image_object = new VulkanImageObject();
		new_image_object->SetVulkanImage(this, new_image, new_allocation, format, width, height, depth, mip_level_num, layer_num, usages, memory_type, VK_SHARING_MODE_EXCLUSIVE);
		return new_image_object;
	}
	void VulkanGPUDevice::HandleDisposedImage(VulkanImageObject* p_image)
	{
		vmaDestroyImage(m_MemoryAllocator, p_image->m_Image, p_image->m_Allocation);
		delete p_image;
	}
	PRenderPass VulkanGPUDevice::CreateRenderPass()
	{
		return new VulkanRenderPass{this};
	}
	void VulkanGPUDevice::HandleDisposedRenderPass(VulkanRenderPass* p_renderpass)
	{
		delete p_renderpass;
	}
	PDescriptorSetLayout VulkanGPUDevice::CreateDescriptorSetLayout()
	{
		return new VulkanDescriptorSetLayout(this);
	}
	void VulkanGPUDevice::HandleDisposedDescriptorSetLayout(VulkanDescriptorSetLayout* p_set_layout)
	{
		delete p_set_layout;
	}
	PGraphicsPipeline VulkanGPUDevice::CreateGraphicsPipeline()
	{
		return new VulkanGraphicsPipeline(this);
	}
	void VulkanGPUDevice::HandleDisposedGraphicsPipeline(VulkanGraphicsPipeline* p_pipeline)
	{
		delete p_pipeline;
	}
	PRayTracer VulkanGPUDevice::CreateRayTracer()
	{
		return new VulkanRayTracer(this);
	}
	void VulkanGPUDevice::HandleDisposedRayTracer(VulkanRayTracer* p_raytracer)
	{
		delete p_raytracer;
	}
	PFramebuffer VulkanGPUDevice::CreateFramebuffer()
	{
		return new VulkanFramebuffer(this);
	}
	void VulkanGPUDevice::HandleDisposedFramebuffer(VulkanFramebuffer* p_framebuffer)
	{
		delete p_framebuffer;
	}
	PRenderPassInstance VulkanGPUDevice::CreateRenderPassInstance(PRenderPass render_pass, PFramebuffer framebuffer)
	{
		VulkanRenderPassInstance* return_val = new VulkanRenderPassInstance();
		return_val->InitRenderPassInstance(this, 
			static_cast<VulkanRenderPass*>(render_pass),
			static_cast<VulkanFramebuffer*>(framebuffer), m_RenderPassInstanceIdPool.Allocate());
		return return_val;
	}
	void VulkanGPUDevice::HandleDisposedRenderPassInstance(VulkanRenderPassInstance* p_render_pass_instance)
	{
		m_RenderPassInstanceIdPool.Recycle(p_render_pass_instance->m_InstanceUniqueId);
		delete p_render_pass_instance;
	}
	PRayTraceGeometry VulkanGPUDevice::CreateRayTraceGeometry()
	{
		VulkanRayTraceGeometry* new_geometry = new VulkanRayTraceGeometry(this);
		return new_geometry;
	}
	void VulkanGPUDevice::HandleDisposedRayTraceGeometry(VulkanRayTraceGeometry* p_geometry)
	{
		delete p_geometry;
	}
	PAccelerationStructure VulkanGPUDevice::CreateAccelerationStructure()
	{ 
		VulkanAccelerationStructure* new_acceleration_structure = new VulkanAccelerationStructure(this);
		return new_acceleration_structure;
	}
	void VulkanGPUDevice::HandleDisposedAccelerationStructure(VulkanAccelerationStructure* p_structure)
	{
		delete p_structure;
	}
	void VulkanGPUDevice::CreateBatch(std::string const& batch_name, EExecutionCommandType command_type, uint32_t priority)
	{
		VulkanBatch* new_batch = new VulkanBatch();
		uint32_t queue_family_id = 0;
		switch (command_type)
		{
		case EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS:
			queue_family_id = m_GraphicsDedicateFamily;
			break;
		case EExecutionCommandType::E_COMMAND_TYPE_COMPUTE:
			queue_family_id = m_ComputeDedicateFamily;
			break;
		default:
			break;
		}
		new_batch->SetBatch(this, m_BatchIdPool.Allocate(), queue_family_id, priority);
		m_Batches.insert(std::make_pair(batch_name, new_batch));
	}
	void VulkanGPUDevice::DestroyBatch(std::string const& batch_name)
	{
		auto find = m_Batches.find(batch_name);
		if (find != m_Batches.end())
		{
			find->second->DestroyBatch();
			m_BatchIdPool.Recycle(find->second->m_BatchID);
			delete find->second;
			m_Batches.erase(find);
		}
	}
	void VulkanGPUDevice::ExecuteBatches(std::vector<std::string> const& batches)
	{
		for (auto& batch_name : batches)
		{
			auto batch_find = m_Batches.find(batch_name);
			if (batch_find != m_Batches.end())
			{
				batch_find->second->SubmitCommands();
			}
		}
	}
	void VulkanGPUDevice::PresentWindow(IWindow& window)
	{
		auto find = m_SurfaceContexts.find(window.GetName());
		if (find != m_SurfaceContexts.end())
		{
			VkResult result;
			VkPresentInfoKHR present_info{};
			present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			present_info.swapchainCount = 1;
			present_info.pSwapchains = &find->second.m_Swapchain;
			present_info.pImageIndices = &find->second.m_PresentImageId;
			present_info.pResults = &result;

			present_info.waitSemaphoreCount = 0;
			present_info.pWaitSemaphores = nullptr;
			present_info.pNext = nullptr;
			vkQueuePresentKHR(find->second.m_PresentQueue, &present_info);
			CHECK_VKRESULT(result, "Vulkan Present Issue!");
		}
	}
	std::vector<VkCommandBuffer> VulkanGPUDevice::CollectSubpassCommandBuffers(uint32_t subpass_id, VulkanRenderPassInstance* p_instance)
	{
		std::vector<VkCommandBuffer> return_val;
		for (auto& thread : m_Threads)
		{
			thread->PushBackSubpassCommandBuffer(return_val, p_instance->m_InstanceUniqueId, subpass_id);
		}
		return return_val;
	}
	std::vector<VkCommandBuffer> VulkanGPUDevice::CollectBatchCommandBuffers(uint32_t batch_id, std::vector<VkSemaphore>& waiting_semaphores, std::vector<VkPipelineStageFlags>& waiting_stages)
	{
		std::vector<VkCommandBuffer> return_val;
		for (auto& thread : m_Threads)
		{
			thread->PushBackExecutionCommandBuffers(return_val, batch_id, waiting_semaphores, waiting_stages);
		}
		return return_val;
	}
	uint32_t VulkanGPUDevice::GetQueueFamilyIdByCommandType(EExecutionCommandType command_type)
	{
		switch (command_type)
		{
		case Crimson::EExecutionCommandType::E_COMMAND_TYPE_GENERAL:
			return m_GraphicsComputeGeneralFamily;
			break;
		case Crimson::EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS:
			return m_GraphicsDedicateFamily;
			break;
		case Crimson::EExecutionCommandType::E_COMMAND_TYPE_COMPUTE:
			return m_ComputeDedicateFamily;
			break;
		case Crimson::EExecutionCommandType::E_COMMAND_TYPE_MAX:
			break;
		default:
			break;
		}
		return NUMMAX_UINT32;
	}
	VulkanGPUDevice::VulkanGPUDevice():
		m_PhysicalDevice(VK_NULL_HANDLE),
		m_LogicalDevice(VK_NULL_HANDLE),
		m_GraphicsComputeGeneralFamily((std::numeric_limits<uint32_t>::max)()),
		m_GraphicsDedicateFamily((std::numeric_limits<uint32_t>::max)()),
		m_ComputeDedicateFamily((std::numeric_limits<uint32_t>::max)()),
		m_TransferDedicateFamily((std::numeric_limits<uint32_t>::max)()),
		m_SparseBindingFamily((std::numeric_limits<uint32_t>::max)()),
		m_DescriptorPool(VK_NULL_HANDLE),
		m_MemoryAllocator(VK_NULL_HANDLE)
	{}
	VulkanGPUDevice::~VulkanGPUDevice()
	{
	}
	void VulkanGPUDevice::InitVulkanDevice(uint32_t prefered_device_index, uint32_t prefered_graphics_queue_num, uint32_t prefered_compute_queue_num, uint32_t prefered_transfer_queue_num)
	{
		auto p_vulkan_instance = VulkanInstance::Get();
		auto& devices = p_vulkan_instance->GetPhysicalDevices();
		//TODO: Check device number
		uint32_t physical_device_index = (std::min)({ prefered_device_index, static_cast<uint32_t>(devices.size() - 1) });
		m_PhysicalDevice = devices[physical_device_index];

		uint32_t family_property_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(devices[physical_device_index], &family_property_count, nullptr);
		m_QueueFamilies.resize(family_property_count);
		m_AllQueueFamilyIds.resize(family_property_count);
		std::vector<uint32_t> queue_numbers(family_property_count, 0);
		vkGetPhysicalDeviceQueueFamilyProperties(devices[physical_device_index], &family_property_count, m_QueueFamilies.data());

		for (uint32_t family_id = 0; family_id < family_property_count; ++family_id)
		{
			m_AllQueueFamilyIds[family_id] = family_id;
			VkQueueFamilyProperties& family_property = m_QueueFamilies[family_id];
			if ((family_property.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) == (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
			{
				//general queue family
				m_GraphicsComputeGeneralFamily = family_id;
			}
			else 
			{
				if (family_property.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					//graphics dedicate
					m_GraphicsDedicateFamily = family_id;
				}
				else if (family_property.queueFlags & VK_QUEUE_COMPUTE_BIT)
				{
					//compute dedicate
					m_ComputeDedicateFamily = family_id;
				}
				else if (family_property.queueFlags & VK_QUEUE_TRANSFER_BIT)
				{
					//transfer dedicate
					m_TransferDedicateFamily = family_id;
				}
			}
		}
		//Assign "dedicate" families with general family if they are empty
		if (m_GraphicsComputeGeneralFamily != (std::numeric_limits<uint32_t>::max()))
		{
			if (m_GraphicsDedicateFamily == (std::numeric_limits<uint32_t>::max()))
			{
				m_GraphicsDedicateFamily = m_GraphicsComputeGeneralFamily;
			}
			if (m_ComputeDedicateFamily == (std::numeric_limits<uint32_t>::max()))
			{
				m_ComputeDedicateFamily = m_GraphicsComputeGeneralFamily;
			}
			if (m_TransferDedicateFamily == (std::numeric_limits<uint32_t>::max()))
			{
				m_TransferDedicateFamily = m_GraphicsComputeGeneralFamily;
			}
		}
		if (m_GraphicsDedicateFamily != (std::numeric_limits<uint32_t>::max()))
		{
			queue_numbers[m_GraphicsDedicateFamily] = (std::max)({ queue_numbers[m_GraphicsDedicateFamily], prefered_graphics_queue_num });
		}
		if (m_ComputeDedicateFamily != (std::numeric_limits<uint32_t>::max()))
		{
			queue_numbers[m_ComputeDedicateFamily] = (std::max)({ queue_numbers[m_GraphicsDedicateFamily], prefered_compute_queue_num });
		}
		if (m_TransferDedicateFamily != (std::numeric_limits<uint32_t>::max()))
		{
			queue_numbers[m_TransferDedicateFamily] = (std::max)({ queue_numbers[m_GraphicsDedicateFamily], prefered_transfer_queue_num });
		}

		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
		std::deque<std::vector<float>> queue_priorities;
		for (uint32_t family_id = 0; family_id < family_property_count; ++family_id)
		{
			queue_numbers[family_id] = (std::min)({ m_QueueFamilies[family_id].queueCount, queue_numbers[family_id] });
			if (queue_numbers[family_id] > 0)
			{
				queue_priorities.push_back(std::vector<float>(queue_numbers[family_id]));
				std::vector<float>& current_queue_priority_list = *(queue_priorities.rbegin());
				{
					if (queue_numbers[family_id] == 1)
					{
						current_queue_priority_list[0] = 1.0f;
					}
					else
					{
						for (uint32_t priority_id = 0; priority_id <= queue_numbers[family_id] - 1; ++priority_id)
						{
							current_queue_priority_list[priority_id] = 1.0f - (priority_id * 1.0f / (queue_numbers[family_id] - 1));
						}
					}
				}
				VkDeviceQueueCreateInfo new_create_info{};
				new_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				new_create_info.queueFamilyIndex = family_id;
				new_create_info.queueCount = queue_numbers[family_id];
				new_create_info.pQueuePriorities = current_queue_priority_list.data();
				new_create_info.pNext = nullptr;
				queue_create_infos.push_back(new_create_info);
			}
		}

		VkPhysicalDeviceFeatures device_features{};
		vkGetPhysicalDeviceFeatures(devices[physical_device_index], &device_features);

		std::vector<char const*> device_extension_names = GetFilteredDeviceExtensions(devices[physical_device_index], {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,

			VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
			VK_NV_RAY_TRACING_EXTENSION_NAME,
			VK_KHR_MAINTENANCE3_EXTENSION_NAME,
		});


		VkDeviceCreateInfo logical_device_create_info = {};
		logical_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		logical_device_create_info.pNext = nullptr;
		logical_device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
		logical_device_create_info.pQueueCreateInfos = queue_create_infos.data();
		logical_device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extension_names.size());
		logical_device_create_info.ppEnabledExtensionNames = device_extension_names.data();
		logical_device_create_info.enabledLayerCount = 0;
		logical_device_create_info.ppEnabledLayerNames = nullptr;
		logical_device_create_info.pEnabledFeatures = &device_features;

		CHECK_VKRESULT(vkCreateDevice(devices[physical_device_index], &logical_device_create_info, VULKAN_ALLOCATOR_POINTER, &m_LogicalDevice), "Vulkan Logical Device Creation Error!");
		std::swap(m_QueueNumbers, queue_numbers);
		InitMemoryAllocator();
		InitDescriptorPool();
		m_NVExtension.InitExtensions(m_LogicalDevice);
	}

	std::vector<char const*> VulkanGPUDevice::GetFilteredDeviceExtensions(VkPhysicalDevice physical_devices, std::vector<char const*> const& wanted_extensions)
	{
		uint32_t ext_count = 0;
		CHECK_VKRESULT(vkEnumerateDeviceExtensionProperties(physical_devices, nullptr, &ext_count, nullptr), "Vulkan Enumerate Device Extensions Issue!");
		std::vector<VkExtensionProperties> properties(ext_count);
		CHECK_VKRESULT(vkEnumerateDeviceExtensionProperties(physical_devices, nullptr, &ext_count, properties.data()), "Vulkan Enumerate Device Extensions Issue!");
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

	void VulkanGPUDevice::InitMemoryAllocator()
	{
		VmaAllocatorCreateInfo allocator_create_info = {};
		allocator_create_info.physicalDevice = m_PhysicalDevice;
		allocator_create_info.device = m_LogicalDevice;
		CHECK_VKRESULT(vmaCreateAllocator(&allocator_create_info, &m_MemoryAllocator), "Vulkan Memory Allocator Initialization Failed!");
	}
	void VulkanGPUDevice::InitDescriptorPool()
	{
		std::vector<VkDescriptorPoolSize> pool_sizes(6);
		// Uniform Buffer Max Size
		pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_sizes[0].descriptorCount = 5000;
		// Combined Image Sampler Max Size
		pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		pool_sizes[1].descriptorCount = 5000;
		// Attachment Storage Image Max Size
		pool_sizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		pool_sizes[2].descriptorCount = 1000;
		// Attachment Input Max Size
		pool_sizes[3].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		pool_sizes[3].descriptorCount = 1000;
		// Attachment SB Max Size
		pool_sizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		pool_sizes[4].descriptorCount = 5000;
		// Acceleration Structure Max Size
		pool_sizes[5].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
		pool_sizes[5].descriptorCount = 100;
		VkDescriptorPoolCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		create_info.maxSets = 1000;
		create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		create_info.pPoolSizes = pool_sizes.data();
		create_info.pNext = nullptr;
		CHECK_VKRESULT(vkCreateDescriptorPool(m_LogicalDevice, &create_info, VULKAN_ALLOCATOR_POINTER, &m_DescriptorPool), "Vulkan Descriptor Pool Creation Issue!");
	}
	void NVExtension::InitExtensions(VkDevice device)
	{
#ifndef GET_EXTENSION_FUNC
#define GET_EXTENSION_FUNC(func_name) {func_name = reinterpret_cast<PFN_##func_name>(vkGetDeviceProcAddr(device, #func_name));}
#endif
		GET_EXTENSION_FUNC(vkCreateAccelerationStructureNV);
		GET_EXTENSION_FUNC(vkDestroyAccelerationStructureNV);
		GET_EXTENSION_FUNC(vkBindAccelerationStructureMemoryNV);
		GET_EXTENSION_FUNC(vkGetAccelerationStructureHandleNV);
		GET_EXTENSION_FUNC(vkGetAccelerationStructureMemoryRequirementsNV);
		GET_EXTENSION_FUNC(vkCmdBuildAccelerationStructureNV);
		GET_EXTENSION_FUNC(vkCreateRayTracingPipelinesNV);
		GET_EXTENSION_FUNC(vkGetRayTracingShaderGroupHandlesNV);
		GET_EXTENSION_FUNC(vkCmdTraceRaysNV);
	}
}
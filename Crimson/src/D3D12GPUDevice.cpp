#include <headers/D3D12GPUDevice.h>
#include <headers/D3D12DebugLog.h>
#include <headers/D3D12Buffer.h>
#include <headers/D3D12Image.h>
#include <headers/D3D12ShaderModule.h>
#include <headers/D3D12RenderPass.h>
#include <headers/D3D12Pipeline.h>
#include <headers/D3D12Descriptors.h>
#include <headers/D3D12Framebuffer.h>
#include <headers/D3D12GPUDeviceThread.h>
#include <headers/D3D12RenderPassInstance.h>
#include <dxcapi.h>
namespace Crimson
{
	void D3D12GPUDevice::RegisterWindow(IWindow& window)
	{
		auto find = m_SurfaceContexts.find(window.GetName());
		if (find == m_SurfaceContexts.end())
		{
			D3D12SurfaceContext newContext{};
#ifdef _WIN32
			newContext.InitSurfaceContext(this, window);
#endif
			m_SurfaceContexts.insert(std::make_pair(window.GetName(), newContext));
		}
	}
	PGPUDeviceThread D3D12GPUDevice::CreateThread()
	{
		D3D12GPUDeviceThread* new_thread = new D3D12GPUDeviceThread();
		new_thread->InitGPUDeviceThread(this);
		m_Threads.push_back(new_thread);
		return new_thread;
	}
	PGPUBuffer D3D12GPUDevice::CreateBuffer(uint64_t buffer_size, std::vector<EBufferUsage> const& usages, EMemoryType memory_type)
	{
		D3D12BufferObject* return_val = new D3D12BufferObject();
		return_val->SetD3D12Buffer(this, buffer_size, usages, memory_type);
		return return_val;
	}
	PGPUImage D3D12GPUDevice::CreateImage(EFormat format, uint32_t width, uint32_t height, uint32_t depth, std::vector<EImageUsage> const& usages, EMemoryType memory_type, uint32_t layer_num, uint32_t mip_level_num, uint32_t sample_num)
	{
		D3D12ImageObject* return_val = new D3D12ImageObject();
		return_val->SetD3D12Image(this, format, width, height, depth, mip_level_num, layer_num, usages, memory_type);
		return return_val;
	}
	PRenderPass D3D12GPUDevice::CreateRenderPass()
	{
		D3D12RenderPass* return_val = new D3D12RenderPass();
		return_val->InitRenderPass(this);
		return return_val;
	}
	PDescriptorSetLayout D3D12GPUDevice::CreateDescriptorSetLayout()
	{
		D3D12DescriptorSetLayout* return_val = new D3D12DescriptorSetLayout(this);
		return return_val;
	}
	PShaderModule D3D12GPUDevice::CreateShaderModule(void* data, size_t size, EShaderType shader_type)
	{
		ComPtr<ID3DBlob> blob;
		D3DCreateBlob(size, &blob);
		memcpy(blob->GetBufferPointer(), data, size);
		D3D12ShaderModule* return_val = new D3D12ShaderModule();
		return_val->Init(this, blob);
		return return_val;
	}
	PGraphicsPipeline D3D12GPUDevice::CreateGraphicsPipeline()
	{
		D3D12GraphicsPipeline* new_pipeline = new D3D12GraphicsPipeline(this);
		return new_pipeline;
	}
	PFramebuffer D3D12GPUDevice::CreateFramebuffer()
	{
		D3D12Framebuffer* new_framebuffer = new D3D12Framebuffer();
		new_framebuffer->Init(this);
		return new_framebuffer;
	}
	PRenderPassInstance D3D12GPUDevice::CreateRenderPassInstance(PRenderPass render_pass, PFramebuffer framebuffer)
	{
		D3D12RenderPassInstance* new_instance = new D3D12RenderPassInstance();
		new_instance->Init(this, static_cast<D3D12RenderPass*>(render_pass), static_cast<D3D12Framebuffer*>(framebuffer));
		return new_instance;
	}
	void D3D12GPUDevice::CreateBatch(std::string const& batch_name, EExecutionCommandType command_type, uint32_t priority)
	{
		if (m_BatchIdMap.find(batch_name) == m_BatchIdMap.end())
		{
			uint32_t id = m_BatchIdMap.size();
			if (!m_AvailableBatchIds.empty())
			{
				id = m_AvailableBatchIds.front();
				m_AvailableBatchIds.pop_front();
			}
			m_BatchIdMap.insert(std::make_pair(batch_name, id));
		}
	}
	void D3D12GPUDevice::ExecuteBatches(std::vector<std::string> const& batches, EExecutionCommandType command_type, uint32_t queue_id)
	{
		ComPtr<ID3D12CommandQueue> queue = nullptr;
		switch (command_type)
		{
		case EExecutionCommandType::E_COMMAND_TYPE_GENERAL:
		case EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS:
			queue = m_GraphicsQueues[std::min(queue_id, static_cast<uint32_t>(m_GraphicsQueues.size() - 1))];
			break;
		case EExecutionCommandType::E_COMMAND_TYPE_COMPUTE:
			queue = m_ComputeQueues[std::min(queue_id, static_cast<uint32_t>(m_ComputeQueues.size() - 1))];
			break;
		case EExecutionCommandType::E_COMMAND_TYPE_COPY:
			queue = m_TransferQueues[std::min(queue_id, static_cast<uint32_t>(m_TransferQueues.size() - 1))];
			break;
		default:
			break;
		}
		std::vector<ID3D12CommandList *const> cmdList;
		for (auto& name : batches)
		{
			auto find = m_BatchIdMap.find(name);
			if (find != m_BatchIdMap.end())
			{
				for (auto pthread : m_Threads)
				{
					if (pthread->m_BatchDataList.size() > find->second)
					{
						pthread->m_BatchDataList[find->second].CollectCmdLists(command_type, cmdList);
					}
				}
			}
		}
		if (!cmdList.empty())
		{
			queue->ExecuteCommandLists(cmdList.size(), cmdList.data());
		}
	}
	void D3D12GPUDevice::PresentWindow(IWindow& window)
	{
		auto find = m_SurfaceContexts.find(window.GetName());
		if (find != m_SurfaceContexts.end())
		{
			UINT syncInterval = find->second.g_VSync ? 1 : 0;
			UINT presentFlags = find->second.g_TearingSupported && ! find->second.g_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
			find->second.swapChain4->Present(syncInterval, presentFlags);
		}
	}
	void D3D12GPUDevice::CollectSubpassCommandLists(D3D12RenderPassInstance* renderpass_instance, std::vector<ComPtr<ID3D12GraphicsCommandList4>>& subpassList, uint32_t subpass_id)
	{
		for (auto thread : m_Threads)
		{
			ComPtr<ID3D12GraphicsCommandList4> result = thread->GetSubpassCommandList(renderpass_instance, subpass_id);
			if (result != nullptr)
			{
				subpassList.push_back(result);
			}
		}
	}
	void D3D12GPUDevice::InitD3D12Device(ComPtr<IDXGIAdapter4> p_adapter, uint32_t prefered_graphics_queue_num, uint32_t prefered_compute_queue_num, uint32_t prefered_transfer_queue_num)
	{
		p_Adapter = p_adapter;
		CHECK_DXRESULT(D3D12CreateDevice(p_Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device)), "DX12 Device Creation Issue!");

		m_GraphicsQueues.resize(prefered_graphics_queue_num);
		m_ComputeQueues.resize(prefered_compute_queue_num);
		m_TransferQueues.resize(prefered_transfer_queue_num);

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		for (uint32_t i = 0; i < prefered_graphics_queue_num; ++i)
		{
			CHECK_DXRESULT(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_GraphicsQueues[i])), "DX12 Command Queue Creation Issue!");
		}

		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		for (uint32_t i = 0; i < prefered_compute_queue_num; ++i)
		{
			CHECK_DXRESULT(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_ComputeQueues[i])), "DX12 Command Queue Creation Issue!");
		}

		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		for (uint32_t i = 0; i < prefered_transfer_queue_num; ++i)
		{
			CHECK_DXRESULT(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_TransferQueues[i])), "DX12 Command Queue Creation Issue!");
		}

		InitDescriptorHeaps();
	}
	void D3D12GPUDevice::InitDescriptorHeaps()
	{
		//D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
		//HeapDesc.NumDescriptors = 100;
		//HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		//HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		//CHECK_DXRESULT(m_Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&m_DescriptorHeaps.m_CBV_SRV_UAV_Heap)), "DX12 Create CBV_SRV_UAV Heap Issue!");
		m_DescriptorHeaps.m_Heaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].Init(m_Device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 100, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
		//m_DescriptorHeaps.m_CBV_SRV_UAV_Heap.Init(m_Device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 100, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		//HeapDesc.NumDescriptors = 100;
		//HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		//HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		//CHECK_DXRESULT(m_Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&m_DescriptorHeaps.m_SamplerHeap)), "DX12 Create Sampler Heap Issue!");
		m_DescriptorHeaps.m_Heaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].Init(m_Device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 100, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
		//m_DescriptorHeaps.m_SamplerHeap.Init(m_Device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 100, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		//HeapDesc.NumDescriptors = 100;
		//HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		//HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		//CHECK_DXRESULT(m_Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&m_DescriptorHeaps.m_RtvHeap)), "DX12 Create Render Target Heap Issue!");
		m_DescriptorHeaps.m_Heaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].Init(m_Device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 100, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
		//m_DescriptorHeaps.m_RtvHeap.Init(m_Device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 100, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		//HeapDesc.NumDescriptors = 100;
		//HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		//HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		//CHECK_DXRESULT(m_Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&m_DescriptorHeaps.m_DsvHeap)), "DX12 Create DSV Heap Issue!");
		m_DescriptorHeaps.m_Heaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].Init(m_Device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 100, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
		//m_DescriptorHeaps.m_DsvHeap.Init(m_Device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 100, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	}
}
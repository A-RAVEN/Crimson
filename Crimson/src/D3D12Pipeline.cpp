#include <headers/D3D12Pipeline.h>
#include <headers/D3D12Translator.h>
#include <headers/D3D12ShaderModule.h>
#include <headers/D3D12DebugLog.h>
#include <map>

namespace Crimson
{
	D3D12GraphicsPipeline::D3D12GraphicsPipeline(D3D12GPUDevice* device)
	{

	}
	void D3D12GraphicsPipeline::LoadShaderSource(char const* src_code, size_t src_size, EShaderType shader_type)
	{
	}
	void D3D12GraphicsPipeline::LoadShaderModule(PShaderModule shader_module)
	{
	}

	void D3D12GraphicsPipeline::Dispose()
	{
	}

	void D3D12GraphicsPipeline::BuildPipeline()
	{
		m_PipelineStateStreamingData.Clear();
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY topology = D3D12TopologyType(m_Topology);
		m_PipelineStateStreamingData.PushData(topology);
		//attribute inputs
		{
			std::map<std::string, uint32_t> sematics_counter;
			m_VertexInputDescriptors.clear();
			uint32_t slot = 0;
			for (auto& input : m_VertexInputs)
			{
				D3D12_INPUT_CLASSIFICATION inputClassification = input.m_VertexInputMode == EVertexInputMode::E_VERTEX_INPUT_PER_VERTEX ? D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA : D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
				uint32_t instanceStepRate = input.m_VertexInputMode == EVertexInputMode::E_VERTEX_INPUT_PER_VERTEX ? 0 : 1;

				uint32_t sematics_size = input.m_Sematics.size();
				for (uint32_t i = 0; i < input.m_DataTypes.size(); ++i)
				{
					D3D12VertexInputDataTypeInfo const& info = D3D12VertexInputDataType(input.m_DataTypes[i]);
					std::string sematic = "";
					if (i < sematics_size)
					{
						sematic = input.m_Sematics[i];
					}
					else
					{
						sematic = info.m_DefaultSematicString;
					}
					auto find = sematics_counter.find(sematic);
					if (find == sematics_counter.end())
					{
						sematics_counter.insert(std::make_pair(sematic, 0));
						find = sematics_counter.find(sematic);
					}
					D3D12_INPUT_ELEMENT_DESC new_input = { find->first.c_str(), find->second, info.m_Format, slot, D3D12_APPEND_ALIGNED_ELEMENT, inputClassification, instanceStepRate };
					m_VertexInputDescriptors.push_back(new_input);
					find->second = find->second + 1;
				}
				++slot;
			}
			CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT inputLayout = D3D12_INPUT_LAYOUT_DESC{ m_VertexInputDescriptors.data(), static_cast<UINT>(m_VertexInputDescriptors.size()) };
			m_PipelineStateStreamingData.PushData(inputLayout);
		}
		//root signature
		{
			//Create one root signature based on descriptor set layouts to simulate vulkan feature
			std::vector<CD3DX12_ROOT_PARAMETER1> parameters;
			for (auto& pair : m_DescriptorSetLayouts)
			{
				//This part can partialy moved into descriptor set layout initialization
				//treat set id as space
				uint32_t space = pair.first;
				PDescriptorSetLayout layout = pair.second;
				std::map<D3D12_DESCRIPTOR_RANGE_TYPE, std::vector<ShaderBinding*>> bindingMap;
				for (auto& shader_binding : layout->m_Bindings)
				{
					auto& info = D3D12ResourceTypeInfo(shader_binding.m_ResourceType);
					auto find = bindingMap.find(info.descriptorRange);
					if (find == bindingMap.end())
					{
						bindingMap.insert(std::make_pair(info.descriptorRange, std::vector<ShaderBinding*>{}));
						find = bindingMap.find(info.descriptorRange);
					}
					find->second.push_back(&shader_binding);
				}
				//
				std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges;
				{
					auto find = bindingMap.find(D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV);
					if (find != bindingMap.end())
					{
						CD3DX12_DESCRIPTOR_RANGE1 new_range{};
						new_range.Init(find->first, find->second.size(), 0, space);
						ranges.push_back(new_range);
					}
				}
				{
					auto find = bindingMap.find(D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
					if (find != bindingMap.end())
					{
						CD3DX12_DESCRIPTOR_RANGE1 new_range{};
						new_range.Init(find->first, find->second.size(), 0, space);
						ranges.push_back(new_range);
					}
				}
				{
					auto find = bindingMap.find(D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
					if (find != bindingMap.end())
					{
						CD3DX12_DESCRIPTOR_RANGE1 new_range{};
						new_range.Init(find->first, find->second.size(), 0, space);
						ranges.push_back(new_range);
					}
				}
				{
					auto find = bindingMap.find(D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER);
					if (find != bindingMap.end())
					{
						CD3DX12_DESCRIPTOR_RANGE1 new_range{};
						new_range.Init(find->first, find->second.size(), 0, space);
						ranges.push_back(new_range);
					}
				}
				CD3DX12_ROOT_PARAMETER1 new_param{};
				//shader visibility set to all for now
				new_param.InitAsDescriptorTable(ranges.size(), ranges.data());
				parameters.push_back(new_param);
			}
			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
			rootSignatureDescription.Init_1_1(parameters.size(), parameters.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);
			ComPtr<ID3DBlob> rootSignatureBlob;
			ComPtr<ID3DBlob> errorBlob;
			CHECK_DXRESULT(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
				D3D_ROOT_SIGNATURE_VERSION_1_1, //Check for version in device initialization
				&rootSignatureBlob, &errorBlob), "D3D12 Error: Serialize Root Signature Issue!");
			// Create the root signature.
			CHECK_DXRESULT(p_OwningDevice->m_Device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
				rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)), "D3D12 Error: Create Root Signature Issue!");
			CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE root_signature_stream = m_RootSignature.Get();
			m_PipelineStateStreamingData.PushData(root_signature_stream);
		}
		//root constants
		//blend state
		{
			CD3DX12_BLEND_DESC  blend_desc{};
			blend_desc.AlphaToCoverageEnable = FALSE;
			blend_desc.IndependentBlendEnable = (m_ColorBlendSettings.size() > 1 || m_AlphaBlendSettings.size() > 1) ? TRUE : FALSE;
			uint32_t blend_state_max = std::min(static_cast<uint32_t>(m_ColorBlendSettings.size()), 8u);
			for (uint32_t i = 0; i < blend_state_max; ++i)
			{
				blend_desc.RenderTarget[i].BlendEnable = (m_ColorBlendSettings[i].IsNoBlendSetting()) ? FALSE : TRUE;
				blend_desc.RenderTarget[i].BlendOp = D3D12BlendOp(m_ColorBlendSettings[i].m_BlendOp);
				blend_desc.RenderTarget[i].SrcBlend = D3D12BlendFactor(m_ColorBlendSettings[i].m_SrcFactor);
				blend_desc.RenderTarget[i].DestBlend = D3D12BlendFactor(m_ColorBlendSettings[i].m_DstFactor);
				blend_desc.RenderTarget[i].LogicOpEnable = FALSE;
				blend_desc.RenderTarget[i].LogicOp = D3D12_LOGIC_OP::D3D12_LOGIC_OP_COPY;
			}

			blend_state_max = std::min(static_cast<uint32_t>(m_AlphaBlendSettings.size()), 8u);
			for (uint32_t i = 0; i < blend_state_max; ++i)
			{
				blend_desc.RenderTarget[i].BlendEnable |= (m_AlphaBlendSettings[i].IsNoBlendSetting()) ? FALSE : TRUE;
				blend_desc.RenderTarget[i].BlendOpAlpha = D3D12BlendOp(m_AlphaBlendSettings[i].m_BlendOp);
				blend_desc.RenderTarget[i].SrcBlendAlpha = D3D12BlendFactor(m_AlphaBlendSettings[i].m_SrcFactor);
				blend_desc.RenderTarget[i].DestBlendAlpha = D3D12BlendFactor(m_AlphaBlendSettings[i].m_DstFactor);
			}
			CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC blend_sream = blend_desc;
			m_PipelineStateStreamingData.PushData(blend_sream);
		}
		//push rasterizer settings
		{
			CD3DX12_RASTERIZER_DESC rasterizer_desc{};
			rasterizer_desc.CullMode = D3D12CullMode(m_CullMode);
			rasterizer_desc.FillMode = D3D12FillMode(m_PolygonMode);
			rasterizer_desc.ForcedSampleCount = m_MultiSampleShadingNum;
			CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER rasterizer_stream = rasterizer_desc;
			m_PipelineStateStreamingData.PushData(rasterizer_stream);
		}
		//depth stencil settings
		{
			CD3DX12_DEPTH_STENCIL_DESC depth_stencil_desc{};
			depth_stencil_desc.DepthEnable = m_DepthRule != EDepthTestRule::E_DEPTH_TEST_DISABLED;
			depth_stencil_desc.StencilEnable = m_StencilRule != EStencilRule::E_STENCIL_DISABLED;
			depth_stencil_desc.DepthFunc == D3D12ComparisonFunc(m_DepthCompareMode);
			depth_stencil_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			switch (m_StencilRule)
			{
			case EStencilRule::E_STENCIL_DISABLED:
				depth_stencil_desc.FrontFace = {};
				depth_stencil_desc.BackFace = {};
				break;
			case EStencilRule::E_STENCIL_WRITE:
				depth_stencil_desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
				depth_stencil_desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP::D3D12_STENCIL_OP_REPLACE;
				depth_stencil_desc.FrontFace.StencilFailOp = D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
				depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
				depth_stencil_desc.StencilReadMask = 0xFF;
				depth_stencil_desc.StencilWriteMask = 0xFF;
				depth_stencil_desc.BackFace = depth_stencil_desc.FrontFace;
				break;
			case EStencilRule::E_STENCIL_TEST:
				depth_stencil_desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
				depth_stencil_desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
				depth_stencil_desc.FrontFace.StencilFailOp = D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
				depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
				depth_stencil_desc.StencilReadMask = 0xff;
				depth_stencil_desc.StencilWriteMask = 0x0;
				depth_stencil_desc.BackFace = depth_stencil_desc.FrontFace;
				break;
			case EStencilRule::E_STENCIL_INVTEST:
				depth_stencil_desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
				depth_stencil_desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
				depth_stencil_desc.FrontFace.StencilFailOp = D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
				depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
				depth_stencil_desc.StencilReadMask = 0xFF;
				depth_stencil_desc.StencilWriteMask = 0x0;
				depth_stencil_desc.BackFace = depth_stencil_desc.FrontFace;
				break;
			}
			CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL depth_stencil_stream = depth_stencil_desc;
			m_PipelineStateStreamingData.PushData(depth_stencil_stream);
		}
		//push shaders
		for (auto p_module : m_ShaderModules)
		{
			D3D12ShaderModule* dxmodule = static_cast<D3D12ShaderModule*>(p_module);
			switch (dxmodule->m_ShaderType)
			{
			case EShaderType::E_SHADER_TYPE_VERTEX:
			{
				CD3DX12_PIPELINE_STATE_STREAM_VS vs = CD3DX12_SHADER_BYTECODE(dxmodule->m_ShaderBlob.Get());
				m_PipelineStateStreamingData.PushData(vs);
			}
			break;
			case EShaderType::E_SHADER_TYPE_FRAGMENT:
			{
				CD3DX12_PIPELINE_STATE_STREAM_PS ps = CD3DX12_SHADER_BYTECODE(dxmodule->m_ShaderBlob.Get());
				m_PipelineStateStreamingData.PushData(ps);
			}
			break;
			case EShaderType::E_SHADER_TYPE_COMPUTE:
			{
				CD3DX12_PIPELINE_STATE_STREAM_CS cs = CD3DX12_SHADER_BYTECODE(dxmodule->m_ShaderBlob.Get());
				m_PipelineStateStreamingData.PushData(cs);
			}
			break;
			case EShaderType::E_SHADER_TYPE_GEOMETRY:
			{
				CD3DX12_PIPELINE_STATE_STREAM_GS gs = CD3DX12_SHADER_BYTECODE(dxmodule->m_ShaderBlob.Get());
				m_PipelineStateStreamingData.PushData(gs);
			}
			break;
			//Tessellation control shader is called hull shader in DX
			case EShaderType::E_SHADER_TYPE_TESSCTR:
			{
				CD3DX12_PIPELINE_STATE_STREAM_HS hs = CD3DX12_SHADER_BYTECODE(dxmodule->m_ShaderBlob.Get());
				m_PipelineStateStreamingData.PushData(hs);
			}
			break;
			//Tessellation evaluation shader is called domain shader in DX
			case EShaderType::E_SHADER_TYPE_TESSEVL:
			{
				CD3DX12_PIPELINE_STATE_STREAM_DS ds = CD3DX12_SHADER_BYTECODE(dxmodule->m_ShaderBlob.Get());
				m_PipelineStateStreamingData.PushData(ds);
			}
			break;
			//Task shader is called amplification shader in DX
			case EShaderType::E_SHADER_TYPE_TASK_NV:
			{
				CD3DX12_PIPELINE_STATE_STREAM_AS as = CD3DX12_SHADER_BYTECODE(dxmodule->m_ShaderBlob.Get());
				m_PipelineStateStreamingData.PushData(as);
			}
			break;
			case EShaderType::E_SHADER_TYPE_MESH_NV:
			{
				CD3DX12_PIPELINE_STATE_STREAM_MS ms = CD3DX12_SHADER_BYTECODE(dxmodule->m_ShaderBlob.Get());
				m_PipelineStateStreamingData.PushData(ms);
			}
			break;
			}
		}
	}
}

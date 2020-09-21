#include <headers/D3D12Pipeline.h>
#include <headers/D3D12Translator.h>
#include <headers/D3D12ShaderModule.h>

namespace Crimson
{
	D3D12GraphicsPipeline::D3D12GraphicsPipeline(D3D12GPUDevice* device)
	{

	}
	void D3D12GraphicsPipeline::LoadShaderModule(PShaderModule shader_module)
	{
	}
	void D3D12GraphicsPipeline::BuildPipeline()
	{
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY topology = D3D12TopologyType(m_Topology);
		m_PipelineStateStreamingData.PushData(topology);
		//push attribute inputs
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
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT inputLayout = D3D12_INPUT_LAYOUT_DESC{ m_VertexInputDescriptors.data(), static_cast<UINT>(m_VertexInputDescriptors.size())};
		m_PipelineStateStreamingData.PushData(inputLayout);
		//push rasterizer settings
		CD3DX12_RASTERIZER_DESC rasterizer_desc{};
		rasterizer_desc.CullMode = D3D12CullMode(m_CullMode);
		rasterizer_desc.FillMode = D3D12FillMode(m_PolygonMode);
		rasterizer_desc.ForcedSampleCount = m_MultiSampleShadingNum;
		CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER rasterizer_stream = rasterizer_desc;
		m_PipelineStateStreamingData.PushData(rasterizer_stream);
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL depth_stencil_stream;
		m_PipelineStateStreamingData.PushData(depth_stencil_stream);
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
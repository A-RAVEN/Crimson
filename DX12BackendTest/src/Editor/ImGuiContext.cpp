#include <headers/Editor/ImGuiContext.h>
#include <external/imgui/imgui.h>
#include <glm/glm.hpp>
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#include <external/imgui/imgui_internal.h>
#endif
#include <headers/ShaderProcessor.h>
using namespace Crimson;

void ImguiContext::InitContext(PGPUDevice device)
{
	m_Device = device;
	m_Layout = m_Device->CreateDescriptorSetLayout();

	m_Layout->m_Bindings.resize(1);
	m_Layout->m_Bindings[0].m_Num = 1;
	m_Layout->m_Bindings[0].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_VERTEX };
	m_Layout->m_Bindings[0].m_ResourceType = EShaderResourceType::E_SHADER_UNIFORM_BUFFER;
	m_Layout->BuildLayout();
	m_Set = m_Layout->AllocDescriptorSet();

	m_ImguiPipeline = m_Device->CreateGraphicsPipeline();
	ShaderProcessor processor;
	{
		auto results = processor.MultiCompile("builtin/imgui.shader");
		for (auto& itr : results)
		{
			m_ImguiPipeline->LoadShaderSource(reinterpret_cast<char*>(itr.second.data()), itr.second.size() * sizeof(uint32_t), itr.first);
		}
		m_ImguiPipeline->m_ColorBlendSettings.resize(1);
		m_ImguiPipeline->m_ColorBlendSettings[0].m_BlendOp = EBlendOp::E_ADD;
		m_ImguiPipeline->m_ColorBlendSettings[0].m_SrcFactor = EBlendFactor::E_SRC_ALPHA;
		m_ImguiPipeline->m_ColorBlendSettings[0].m_DstFactor = EBlendFactor::E_ONE_MINUS_SRC_ALPHA;
		m_ImguiPipeline->m_VertexInputs.resize(1);
		m_ImguiPipeline->m_VertexInputs[0].m_VertexInputMode = EVertexInputMode::E_VERTEX_INPUT_PER_VERTEX;
		m_ImguiPipeline->m_VertexInputs[0].m_DataTypes = { EDataType::EVEC2, EDataType::EVEC2, EDataType::ERGBA8_UNORM };
		m_ImguiPipeline->m_DescriptorSetLayouts = { std::make_pair(0, m_Layout) };
	}

	m_ImguiRenderPass = m_Device->CreateRenderPass();
	m_ImguiRenderPass->m_Attachments.resize(1);
	m_ImguiRenderPass->m_Attachments[0].m_ClearType = EAttachmentClearType::E_ATTACHMENT_NOT_CLEAR;
	m_ImguiRenderPass->m_Attachments[0].m_Format = EFormat::E_FORMAT_B8G8R8A8_SRGB;
	m_ImguiRenderPass->m_Attachments[0].m_SampleCount = 1;
	m_ImguiRenderPass->m_Subpasses.resize(1);
	m_ImguiRenderPass->m_Subpasses[0].m_OutputAttachments = { 0 };
	m_ImguiRenderPass->InstanciatePipeline(m_ImguiPipeline, 0);
}

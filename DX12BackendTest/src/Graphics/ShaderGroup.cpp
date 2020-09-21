#include <headers/Resources/ShaderGroup.h>
#include <headers/Debug.h>
#include <headers/VertexData.h>

//void ShaderGroup::AddShaderModules(std::vector<PShaderModule> const& shader_modules)
//{
//	for (auto pshader : shader_modules)
//	{
//		m_ShaderModules.push_back(pshader);
//	}
//}
//
//void ShaderGroup::FreeShaderGroup()
//{
//	for (auto pshader : m_ShaderModules)
//	{
//		pshader->Dispose();
//	}
//	m_ShaderModules.clear();
//}
//
//ShaderModuleResource::ShaderModuleResource() : m_Shader(nullptr)
//{
//}
//
//bool ShaderModuleResource::Ready()
//{
//	return m_Shader != nullptr;
//}
//
//void ShaderModuleResource::SetShader(std::string const& name, PShaderModule shader)
//{
//	RE_ASSERT(shader != nullptr, "Invalid Added Shader!");
//	RE_ASSERT(m_Shader == nullptr, "Shader Resource Already Assigned With Shader!");
//	m_Name = name;
//	m_Shader = shader;
//}
//
//PShaderModule ShaderModuleResource::GetShaderModule() const
//{
//	return m_Shader;
//}

void PipelinePackageResource::AddGraphicsPipeline(std::string const& name, PGraphicsPipeline pipeline)
{
	//size_t key = std::hash<std::string>{}(name);
	m_PipelineMap.insert(std::make_pair(name, pipeline));
}

void PipelinePackageResource::AddShaderModule(EShaderType shader_type, PShaderModule shader_module)
{

}

void PipelinePackageResource::AddShaderReference(std::string const& package_name, std::string const& shader)
{
}

void PipelinePackageResource::SetPipelineDescriptor(PipelineDescriptor& descriptor)
{
	m_Descriptor = descriptor;
}

void PipelinePackageResource::BuildPipeline()
{
	m_LocalShaders.clear();
	m_LocalShaders.reserve(m_ShaderBytes.size());
	for (auto& result : m_ShaderBytes)
	{
		PShaderModule p_module = m_OwningDevice->CreateShaderModule(result.second.data(), result.second.size() * sizeof(uint32_t), result.first);
		m_LocalShaders.push_back(std::make_pair(result.first, p_module));
	}
	
	GraphicsPipelineInstanciationInfo new_pipeline_instantiactionInfo;
	new_pipeline_instantiactionInfo.m_ColorBlendSettings = m_Descriptor.m_ColorBlendSettings;
	new_pipeline_instantiactionInfo.m_AlphaBlendSettings = m_Descriptor.m_AlphaBlendSettings;
	for (uint32_t i = 0; i < m_Descriptor.m_Shaders.size(); ++i)
	{
		PShaderModule new_module = m_OwningDevice->CreateShaderModule(m_Descriptor.m_Shaders[i].second.data(), m_Descriptor.m_Shaders[i].second.size() * sizeof(uint32_t), m_Descriptor.m_Shaders[i].first);
		new_pipeline_instantiactionInfo.m_Shaders.push_back(std::make_pair(m_Descriptor.m_Shaders[i].first, new_module));
	}
	new_pipeline_instantiactionInfo.m_VertexInputs.resize(2);
	new_pipeline_instantiactionInfo.m_VertexInputs[0].m_DataTypes = VertexDataLightWeight::GetDataType();
	new_pipeline_instantiactionInfo.m_VertexInputs[0].m_VertexInputMode = EVertexInputMode::E_VERTEX_INPUT_PER_VERTEX;
}

PGraphicsPipeline PipelinePackageResource::GetPipeline(std::string const& vertex_type)
{
	auto find = m_PipelineMap.find(vertex_type);
	if (find != m_PipelineMap.end())
	{
		return find->second;
	}
	else
	{

	}
	return nullptr;
}
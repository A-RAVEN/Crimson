#include <headers/Manager/ResourceManager.h>
#include <headers/HelperFunc.h>

void ResourceManager::InitResourceManager(PGPUDevice device)
{
	m_Device = device;

}

void ResourceManager::LoadShaderModule(std::string shader_name)
{
	
}

void ShaderModuleResourceManager::Init(PGPUDevice device)
{
	m_Device = device;
}

void ShaderModuleResourceManager::LoadShaderSource(std::string const& shader_name)
{
	std::string src_code = LoadStringFile(shader_name);
	PipelineDescriptor descriptor;
	CompileResult result = m_ShaderProcessor.MultiCompilePipeline(src_code, descriptor);
	PipelinePackageResource* new_resource = m_Allocator.AllocResource(shader_name);
	new_resource->SetPipelineDescriptor(descriptor);
	for (auto& sub_shader : result)
	{
		PShaderModule new_shader_module = m_Device->CreateShaderModule(sub_shader.second.data(), sub_shader.second.size() * sizeof(uint32_t));
		PShaderModule p_module = m_Device->CreateShaderModule(sub_shader.second.data(), sub_shader.second.size() * sizeof(uint32_t));
		new_resource->AddShaderModule(sub_shader.first, p_module);
	}
}

PipelinePackageResource* ShaderModuleResourceManager::GetPackage(std::string const& package_name)
{
	return m_Allocator.GetResource(package_name);
}

void ShaderModuleResourceManager::Free()
{

}

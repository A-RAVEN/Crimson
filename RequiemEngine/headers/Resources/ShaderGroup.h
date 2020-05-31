#pragma once
#include <Pipeline.h>
#include <headers/Resources/IResource.h>
#include <map>
#include <headers/ShaderProcessor.h>
#include <GPUDevice.h>
using namespace Crimson;
//class ShaderModuleResource : public IResource
//{
//public:
//	ShaderModuleResource();
//	virtual bool Ready() override;
//	void SetShader(std::string const& name, PShaderModule shader);
//	PShaderModule GetShaderModule() const;
//private:
//	PShaderModule m_Shader;
//
//};

//class PipelinePack
//{
//public:
//	std::vector<PGraphicsPipeline> m_Pipelines;
//};

class PipelinePackageResource: public IResource
{
public:
	void Init(PGPUDevice device);
	void AddGraphicsPipeline(std::string const& name, PGraphicsPipeline pipeline);
	void AddShaderModule(EShaderType shader_type, PShaderModule shader_module);
	void AddShaderReference(std::string const& package_name, std::string const& shader);
	void SetPipelineDescriptor(PipelineDescriptor& descriptor);

	void BuildPipeline();
	PGraphicsPipeline GetPipeline(std::string const& vertex_type);

	void AddShaderBytes(CompileResult const& results, std::vector<std::string> const& shader_references);
private:
	CompileResult m_ShaderBytes;
	std::vector<std::string> m_ShaderReferences;
	std::vector<std::pair<EShaderType, PShaderModule>>  m_LocalShaders;

	std::map<std::string, PGraphicsPipeline> m_PipelineMap;
	PipelineDescriptor m_Descriptor;
	std::vector<PDescriptorSetLayout> m_LocalLayouts;
	PGPUDevice m_OwningDevice;
};
//class ShaderGroup
//{
//public:
//	void AddShaderModules(std::vector<PShaderModule> const& shader_modules);
//	void FreeShaderGroup();
//private:
//	std::vector<PShaderModule> m_ShaderModules;
//};
#pragma once
#include <string>
#include <vector>
#include <Pipeline.h>

using namespace Crimson;
class ShaderPack
{
public:
	std::string path;
	std::vector<std::string> shaderPackDependencies;
	std::vector<PShaderModule> localShaderModules;
	std::vector<std::string> localPipelineNames;
	std::vector<PGraphicsPipeline> localPipelines;
};
#pragma once
#include <string>
#include <vector>
#include <Pipeline.h>

using namespace Crimson;
class PipelineResource
{
public:
	std::string m_Name;
	std::string m_ShaderGroupName;
	PGraphicsPipeline m_Pipeline;
};
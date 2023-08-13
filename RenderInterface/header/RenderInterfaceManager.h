#pragma once
#include <memory>
#include "CRenderGraph.h"

class RenderInterfaceManager
{
public:
	virtual std::shared_ptr<CRenderGraph> NewRenderGraph() = 0;

};
#pragma once
#include <memory>
#include "CRenderGraph.h"

namespace graphics_backend
{
	class RenderInterfaceManager
	{
	public:
		virtual std::shared_ptr<CRenderGraph> NewRenderGraph() = 0;
	};
}
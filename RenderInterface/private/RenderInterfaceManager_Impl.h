#pragma once
#include <RenderInterface/header/RenderInterfaceManager.h>

class RenderInterfaceManager_Impl : public RenderInterfaceManager
{
public:
	virtual std::shared_ptr<CRenderGraph> NewRenderGraph() override;
private:
	CRenderGraph* NewRenderGraph_Internal();
	void ReleaseRenderGraph_Internal(CRenderGraph* graph);
};


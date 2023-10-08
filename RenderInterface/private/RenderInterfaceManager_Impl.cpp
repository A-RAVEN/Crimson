#include "pch.h"
#include "RenderInterfaceManager_Impl.h"
#include "CRenderGraphImpl.h"

namespace graphics_backend
{
    std::shared_ptr<CRenderGraph> RenderInterfaceManager_Impl::NewRenderGraph()
    {
        return std::shared_ptr<CRenderGraph>(NewRenderGraph_Internal(), [this](CRenderGraph* pDeleted)
            {
                ReleaseRenderGraph_Internal(pDeleted);
            });
    }

    CRenderGraph* RenderInterfaceManager_Impl::NewRenderGraph_Internal()
    {
        return new CRenderGraph_Impl();
    }

    void RenderInterfaceManager_Impl::ReleaseRenderGraph_Internal(CRenderGraph* pDeleted)
    {
        delete static_cast<CRenderGraph_Impl*>(pDeleted);
    }

    CA_LIBRARY_INSTANCE_LOADING_FUNCTIONS(RenderInterfaceManager, RenderInterfaceManager_Impl)
}


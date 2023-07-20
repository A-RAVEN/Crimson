#pragma once
#include <functional>
#include <vector>

#include "Common.h"
#include "CPipelineStateObject.h"



class CInlineCommandList;
struct CAttachmentInfo
{
	ETextureFormat format = ETextureFormat::E_R8G8B8A8_UNORM;
	EAttachmentLoadOp loadOp = EAttachmentLoadOp::eLoad;
	EAttachmentStoreOp storeOp = EAttachmentStoreOp::eStore;
	EMultiSampleCount multiSampleCount = EMultiSampleCount::e1;
};

struct CSubpassInfo
{
	std::vector<uint32_t> colorAttachmentIDs{};
	std::vector<uint32_t> pixelInputAttachmentIDs{};
	std::vector<uint32_t> preserveAttachmentIDs{};
	uint32_t depthAttachmentIDs = INVALID_INDEX;
};



class CRenderpassBuilder
{
public:
	CRenderpassBuilder(CAttachmentInfo const& inAttachmentInfo);
	CRenderpassBuilder& Subpass(CSubpassInfo const& inSubpassInfo
		, CPipelineStateObject const& pipelineStates
		, std::function<void* (CInlineCommandList&)> commandFunction);
	void Build();
};

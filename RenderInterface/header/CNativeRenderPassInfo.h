#pragma once
#include <functional>
#include <vector>
#include <SharedTools/header/uhash.h>

#include "Common.h"
#include "CPipelineStateObject.h"

constexpr uint32_t INVALID_ATTACHMENT_INDEX = 256;

class CInlineCommandList;
struct CAttachmentInfo
{
	ETextureFormat format = ETextureFormat::E_R8G8B8A8_UNORM;
	EAttachmentLoadOp loadOp = EAttachmentLoadOp::eLoad;
	EAttachmentStoreOp storeOp = EAttachmentStoreOp::eStore;
	EMultiSampleCount multiSampleCount = EMultiSampleCount::e1;
	EAttachmentLoadOp stencilLoadOp = EAttachmentLoadOp::eDontCare;
	EAttachmentStoreOp stencilStoreOp = EAttachmentStoreOp::eDontCare;
};

template<>
struct hash_utils::is_contiguously_hashable<CAttachmentInfo> : public std::true_type {};

struct CSubpassInfo
{
	std::vector<uint32_t> colorAttachmentIDs{};
	std::vector<uint32_t> pixelInputAttachmentIDs{};
	std::vector<uint32_t> preserveAttachmentIDs{};
	uint32_t depthAttachmentID = INVALID_ATTACHMENT_INDEX;

	bool operator==(CSubpassInfo const& rhs) const
	{
		return colorAttachmentIDs == rhs.colorAttachmentIDs
			&& pixelInputAttachmentIDs == rhs.pixelInputAttachmentIDs
			&& preserveAttachmentIDs == rhs.preserveAttachmentIDs
			&& depthAttachmentID == rhs.depthAttachmentID;
	}

	template <class HashAlgorithm>
	friend void hash_append(HashAlgorithm& h, CSubpassInfo const& subpassInfo) noexcept
	{
		hash_append(h, subpassInfo.colorAttachmentIDs);
		hash_append(h, subpassInfo.pixelInputAttachmentIDs);
		hash_append(h, subpassInfo.preserveAttachmentIDs);
		hash_append(h, subpassInfo.depthAttachmentID);
	}
};

struct CRenderPassInfo
{
	std::vector<CAttachmentInfo> attachmentInfos{};
	std::vector<CSubpassInfo> subpassInfos{};

	bool operator==(CRenderPassInfo const& rhs) const
	{
		return attachmentInfos == rhs.attachmentInfos && subpassInfos == rhs.subpassInfos;
	}

	template <class HashAlgorithm>
	friend void hash_append(HashAlgorithm& h, CRenderPassInfo const& renderPassInfo) noexcept
	{
		hash_append(h, renderPassInfo.attachmentInfos);
		hash_append(h, renderPassInfo.subpassInfos);
	}
};

class CRenderpassBuilder
{
public:
	CRenderpassBuilder(CAttachmentInfo const& inAttachmentInfo);
	CRenderpassBuilder& Subpass(CSubpassInfo const& inSubpassInfo
		, CPipelineStateObject const& pipelineStates
		, std::function<void* (CInlineCommandList&)> commandFunction);

	CRenderPassInfo const& GetRenderPassInfo() const { return mRenderPassInfo; }
private:
	CRenderPassInfo mRenderPassInfo{};
	std::vector<std::function<void* (CInlineCommandList&)>> mSubpassFunctions{};
};
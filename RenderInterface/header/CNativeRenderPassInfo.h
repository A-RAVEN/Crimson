#pragma once
#include <functional>
#include <vector>
#include <SharedTools/header/uhash.h>

#include "Common.h"
#include "CPipelineStateObject.h"
#include "ShaderProvider.h"
#include "CVertexInputDescriptor.h"

constexpr uint32_t INVALID_ATTACHMENT_INDEX = 256;

class CInlineCommandList;

struct CAttachmentInfo
{
	ETextureFormat format = ETextureFormat::E_R8G8B8A8_UNORM;
	EAttachmentLoadOp loadOp = EAttachmentLoadOp::eLoad;
	EAttachmentStoreOp storeOp = EAttachmentStoreOp::eStore;
	EAttachmentLoadOp stencilLoadOp = EAttachmentLoadOp::eDontCare;
	EAttachmentStoreOp stencilStoreOp = EAttachmentStoreOp::eDontCare;
	GraphicsClearValue clearValue = GraphicsClearValue::ClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	EMultiSampleCount multiSampleCount = EMultiSampleCount::e1;

	bool operator==(CAttachmentInfo const& rhs) const
	{
		return format == rhs.format
			&& loadOp == rhs.loadOp
			&& storeOp == rhs.storeOp
			&& multiSampleCount == rhs.multiSampleCount
			&& stencilLoadOp == rhs.stencilLoadOp
			&& stencilStoreOp == rhs.stencilStoreOp
			&& clearValue == rhs.clearValue;
	}
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
	CRenderpassBuilder(std::vector<CAttachmentInfo> const& inAttachmentInfo)
	{
		mRenderPassInfo.attachmentInfos = inAttachmentInfo;
	}
	CRenderpassBuilder& Subpass(CSubpassInfo const& inSubpassInfo
		, CPipelineStateObject const& pipelineStates
		, CVertexInputDescriptor const& vertexInputs
		, GraphicsShaderSet const& shaderSet
		, std::function<void(CInlineCommandList&)> commandFunction)
	{
		mRenderPassInfo.subpassInfos.push_back(inSubpassInfo);
		mSubpassPipelineStateObjects.push_back(pipelineStates);
		mSubpassVertexInputs.push_back(vertexInputs);
		mSubpassShaderSets.push_back(shaderSet);
		mSubpassFunctions.push_back(commandFunction);
		return *this;
	}

	CRenderPassInfo const& GetRenderPassInfo() const { return mRenderPassInfo; }
	CPipelineStateObject const& GetPipelineStateObject(uint32_t subpassIndex) const { return mSubpassPipelineStateObjects[subpassIndex]; }
	CVertexInputDescriptor const& GetVertexDescriptor(uint32_t subpassIndex) const { return mSubpassVertexInputs[subpassIndex]; }
	GraphicsShaderSet const& GetShaderSet(uint32_t subpassIndex) const { return mSubpassShaderSets[subpassIndex]; }
	std::function<void(CInlineCommandList&)> GetSubpassFunctor(uint32_t subpassIndex) const { return mSubpassFunctions[subpassIndex]; }
private:
	CRenderPassInfo mRenderPassInfo{};
	std::vector<CPipelineStateObject> mSubpassPipelineStateObjects{};
	std::vector<CVertexInputDescriptor> mSubpassVertexInputs{};
	std::vector<GraphicsShaderSet> mSubpassShaderSets{};
	std::vector<std::function<void(CInlineCommandList&)>> mSubpassFunctions{};
};

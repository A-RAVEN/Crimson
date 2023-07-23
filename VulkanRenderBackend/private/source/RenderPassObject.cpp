#include <private/include/pch.h>
#include <private/include/RenderPassObject.h>
#include <private/include/InterfaceTranslator.h>

namespace graphics_backend
{
	RenderPassObject::RenderPassObject(CVulkanApplication& application) : BaseApplicationSubobject(application)
	{
	}

	void RenderPassObject::Create(RenderPassDescriptor const& descriptor)
	{
		auto& attachmentInfo = descriptor.renderPassInfo.attachmentInfos;
		auto& subpassInfos = descriptor.renderPassInfo.subpassInfos;
		auto& externalInfo = descriptor.externalInfo;

		std::vector<vk::AttachmentDescription> attachmentList{attachmentInfo.size()};
		for (size_t attachmentId = 0; attachmentId < attachmentList.size(); ++attachmentId)
		{
			auto& attachmentDesc = attachmentList[attachmentId];
			auto const& srcInfo = attachmentInfo[attachmentId];
			auto const& externalLayouts = externalInfo.attachmentExternalLayouts[attachmentId];

			attachmentDesc.initialLayout = externalLayouts.first;
			attachmentDesc.finalLayout = externalLayouts.second;
			attachmentDesc.format = ETextureFormatToVkFotmat(srcInfo.format);
			attachmentDesc.samples = EMultiSampleCountToVkSampleCount(srcInfo.multiSampleCount);
			attachmentDesc.loadOp = EAttachmentLoadOpToVkLoadOp(srcInfo.loadOp);
			attachmentDesc.storeOp = EAttachmentStoreOpToVkStoreOp(srcInfo.storeOp);
			attachmentDesc.stencilLoadOp = EAttachmentLoadOpToVkLoadOp(srcInfo.stencilLoadOp);
			attachmentDesc.stencilStoreOp = EAttachmentStoreOpToVkStoreOp(srcInfo.stencilStoreOp);
		}

		std::vector<vk::SubpassDescription> subpassDescs{subpassInfos.size()};
		std::vector<std::vector<vk::AttachmentReference>> subpassAttachmentRefs{};
		std::vector<std::vector<uint32_t>> subpassPreserveAttachmentIDs{};
		std::vector<vk::AttachmentReference> subpassDepthAttachmentRefs{};
		for (size_t subpassId = 0; subpassId < subpassDescs.size(); ++subpassId)
		{
			auto& subpassDesc = subpassDescs[subpassId];
			auto& srcInfo = subpassInfos[subpassId];

			subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;

			subpassAttachmentRefs.emplace_back(srcInfo.colorAttachmentIDs.size());
			auto& attachmentRefs = subpassAttachmentRefs.back();
			for (size_t refId = 0; refId < attachmentRefs.size(); ++refId)
			{
				auto& attachmentRef = attachmentRefs[refId];
				const uint32_t srcRefId = srcInfo.colorAttachmentIDs[refId];
				attachmentRef.attachment = srcRefId;
				attachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;
			};
			subpassDesc.setColorAttachments(attachmentRefs);

			subpassAttachmentRefs.emplace_back(srcInfo.pixelInputAttachmentIDs.size());
			auto& inputAttachmentRefs = subpassAttachmentRefs.back();
			for (size_t refId = 0; refId < inputAttachmentRefs.size(); ++refId)
			{
				auto& attachmentRef = inputAttachmentRefs[refId];
				const uint32_t srcRefId = srcInfo.pixelInputAttachmentIDs[refId];
				attachmentRef.attachment = srcRefId;
				attachmentRef.layout = vk::ImageLayout::eShaderReadOnlyOptimal;
			};
			subpassDesc.setInputAttachments(inputAttachmentRefs);

			if(srcInfo.depthAttachmentID != INVALID_ATTACHMENT_INDEX)
			{
				subpassDepthAttachmentRefs.emplace_back();
				auto& depthAttachmentRef = subpassDepthAttachmentRefs.back();
				depthAttachmentRef.attachment = srcInfo.depthAttachmentID;
				depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
				subpassDesc.setPDepthStencilAttachment(&depthAttachmentRef);
			}

			if (!srcInfo.preserveAttachmentIDs.empty())
			{
				subpassPreserveAttachmentIDs.emplace_back(srcInfo.preserveAttachmentIDs.size());
				auto& preserveAttachmentIDs = subpassPreserveAttachmentIDs.back();
				for (size_t refId = 0; refId < preserveAttachmentIDs.size(); ++refId)
				{
					preserveAttachmentIDs[refId] = srcInfo.preserveAttachmentIDs[refId];
				};
				subpassDesc.setPreserveAttachments(preserveAttachmentIDs);
			}
		}

		vk::RenderPassCreateInfo renderpass_createInfo(
			{}
			, attachmentList
			, subpassDescs
			, {}//TODO Dependencies
		);
		m_RenderPass = GetDevice().createRenderPass(renderpass_createInfo);
	}

	void RenderPassObject::Release()
	{
		if (m_RenderPass != vk::RenderPass(nullptr))
		{
			GetDevice().destroyRenderPass(m_RenderPass);
			m_RenderPass = nullptr;
		}
	}


}
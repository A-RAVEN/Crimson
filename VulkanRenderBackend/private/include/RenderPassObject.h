#pragma once
#include <private/include/VulkanIncludes.h>
#include <RenderInterface/header/CNativeRenderPassInfo.h>
#include <private/include/VulkanApplicationSubobjectBase.h>
#include <SharedTools/header/uhash.h>
#include <unordered_map>
#include <private/include/HashPool.h>

namespace graphics_backend
{
	struct RenderPassDescriptor
	{
		CRenderPassInfo renderPassInfo{};

		bool operator==(RenderPassDescriptor const& other) const noexcept
		{
			return renderPassInfo == other.renderPassInfo;
		}

		template <class HashAlgorithm>
		friend void hash_append(HashAlgorithm& h, RenderPassDescriptor const& renderpass_desc) noexcept
		{
			hash_append(h, renderpass_desc.renderPassInfo);
		}
	};

	class RenderPassObject : public BaseApplicationSubobject
	{
	public:
		RenderPassObject(CVulkanApplication& application);
		void Create(RenderPassDescriptor const& descriptor);
		virtual void Release() override;
		vk::RenderPass GetRenderPass() const { return m_RenderPass; }
		uint32_t GetAttachmentCount() const { return m_AttachmentCounrt; }
		uint32_t GetSubpassCount() const { return m_SubpassCount; }
	private:
		uint32_t m_AttachmentCounrt = 0;
		uint32_t m_SubpassCount = 0;
		vk::RenderPass m_RenderPass = nullptr;
		std::vector<std::pair<vk::ImageLayout, vk::ImageLayout>> m_AttachmentExternalLayouts;
	};

	using RenderPassObjectDic = HashPool<RenderPassDescriptor, RenderPassObject>;
}
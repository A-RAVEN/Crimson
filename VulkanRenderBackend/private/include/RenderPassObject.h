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
		struct SubpassExternalDependencyInfo
		{
			std::vector<std::pair<vk::ImageLayout, vk::ImageLayout>> attachmentExternalLayouts;

			bool operator==(SubpassExternalDependencyInfo const& other) const noexcept
			{
				return attachmentExternalLayouts == other.attachmentExternalLayouts;
			}

			template <class HashAlgorithm>
			friend void hash_append(HashAlgorithm& h, SubpassExternalDependencyInfo const& externalInfo) noexcept
			{
				hash_append(h, externalInfo.attachmentExternalLayouts);
			}
		};
		CRenderPassInfo renderPassInfo{};
		SubpassExternalDependencyInfo externalInfo{};

		bool operator==(RenderPassDescriptor const& other) const noexcept
		{
			return renderPassInfo == other.renderPassInfo && externalInfo == other.externalInfo;
		}

		template <class HashAlgorithm>
		friend void hash_append(HashAlgorithm& h, RenderPassDescriptor const& renderpass_desc) noexcept
		{
			hash_append(h, renderpass_desc.renderPassInfo);
			hash_append(h, renderpass_desc.externalInfo);
		}
	};

	class RenderPassObject : public BaseApplicationSubobject
	{
	public:
		RenderPassObject(CVulkanApplication& application);
		void Create(RenderPassDescriptor const& descriptor);
		virtual void Release() override;
		vk::RenderPass GetRenderPass() const { return m_RenderPass; }
	private:
		vk::RenderPass m_RenderPass = nullptr;
	};

	using RenderPassObjectDic = HashPool<RenderPassDescriptor, vk::RenderPass>;
}
#pragma once
#include <vector>
#include <private/include/VulkanIncludes.h>
#include <private/include/VulkanApplicationSubobjectBase.h>
#include "RenderPassObject.h"
#include <SharedTools/header/uhash.h>

template<>
struct hash_utils::is_contiguously_hashable<vk::ImageView> : public std::true_type {};

namespace graphics_backend
{

	struct FramebufferDescriptor
	{
	public:
		std::vector<vk::ImageView> renderImageViews;
		std::shared_ptr<RenderPassObject> renderpassObject = nullptr;
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t layers = 0;

		bool operator==(FramebufferDescriptor const& rhs) const
		{
			return renderImageViews == rhs.renderImageViews
				&& renderpassObject == rhs.renderpassObject;
		}

		template <class HashAlgorithm>
		friend void hash_append(HashAlgorithm& h, FramebufferDescriptor const& x) noexcept
		{
			hash_append(h, x.renderImageViews);
			hash_append(h, reinterpret_cast<size_t>(x.renderpassObject.get()));
		}
	};

	class FramebufferObject final : public BaseApplicationSubobject
	{
	public:
		FramebufferObject(CVulkanApplication& owner) : BaseApplicationSubobject(owner) {};
		void Create(FramebufferDescriptor const& framebufferDescriptor);
		vk::Framebuffer const& GetFramebuffer() const { return mFramebuffer; }
	private:
		vk::Framebuffer mFramebuffer;
	};

	using FramebufferObjectDic = HashPool<FramebufferDescriptor, FramebufferObject>;
}
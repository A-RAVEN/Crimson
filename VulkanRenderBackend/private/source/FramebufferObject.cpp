#include <private/include/FramebufferObject.h>

namespace graphics_backend
{
	void FramebufferObject::Create(FramebufferDescriptor const& framebufferDescriptor)
	{
		mFramebuffer = GetDevice().createFramebuffer(vk::FramebufferCreateInfo{
			vk::FramebufferCreateFlagBits::eImageless
				, framebufferDescriptor.renderpassObject->GetRenderPass()
				, framebufferDescriptor.renderImageViews
				, framebufferDescriptor.width
				, framebufferDescriptor.height
				, framebufferDescriptor.layers
		});
	}
}
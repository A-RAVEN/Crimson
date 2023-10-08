#pragma once
#include <functional>
#include <string>
#include <vector>
#include <type_traits>
#include "CNativeRenderPassInfo.h"
#include "CTextureHandle.h"
#include "WindowHandle.h"

namespace graphics_backend
{
	struct TextureHandleInternalInfo
	{
	public:
		TIndex m_DescriptorIndex = INVALID_INDEX;
		std::shared_ptr<graphics_backend::WindowHandle> p_WindowsHandle = nullptr;
	};

	class CRenderGraph
	{
	public:
		//Used By Client
		virtual TextureHandle NewTextureHandle(GPUTextureDescriptor const& textureDesc) = 0;
		virtual TextureHandle RegisterWindowBackbuffer(std::shared_ptr<WindowHandle> window) = 0;
		virtual CRenderpassBuilder& NewRenderPass(std::vector<CAttachmentInfo> const& inAttachmentInfo) = 0;
		virtual void PresentWindow(std::shared_ptr<WindowHandle> window) = 0;

		//Used By Backend
		virtual uint32_t GetRenderNodeCount() const = 0;
		virtual CRenderpassBuilder const& GetRenderPass(uint32_t nodeID) const = 0;
		virtual TextureHandle TextureHandleByIndex(TIndex index) const = 0;
		virtual TextureHandleInternalInfo const& GetTextureHandleInternalInfo(TIndex index) const = 0;
		virtual std::shared_ptr<WindowHandle> GetTargetWindow() const = 0;
		virtual TIndex WindowHandleToTextureIndex(std::shared_ptr<WindowHandle> handle) const = 0;

		template<typename TWIN>
		std::shared_ptr<TWIN> GetTargetWindow() const
		{
			static_assert(std::is_base_of<WindowHandle, TWIN>::value, "Invalid WindowHandle Type");
			return std::static_pointer_cast<TWIN>(GetTargetWindow());
		}
	};
}

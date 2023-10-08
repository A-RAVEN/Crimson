#include "private/include/pch.h"
#include "IUploadingResource.h"
#include "CVulkanApplication.h"

namespace graphics_backend
{
	BaseUploadingResource::BaseUploadingResource(CVulkanApplication& owner) : BaseApplicationSubobject(owner)
	{
	}
	void BaseUploadingResource::Upload()
	{
		DoUpload();
	}
	void BaseUploadingResource::UploadAsync(UploadingResourceType resourceType)
	{
		auto task = GetVulkanApplication().NewUploadingTask(resourceType)->Name("Uploading Task");
		task->Functor([this]() {
			DoUpload();
		});
	}
	bool BaseUploadingResource::UploadingDone() const
	{
		if (m_SubmitFrame == INVALID_FRAMEID)
			return false;
		auto& frameContext = GetFrameCountContext();
		if (!frameContext.AnyFrameFinished())
			return false;
		return frameContext.GetReleasedFrameID() >= m_SubmitFrame;
	}
	void BaseUploadingResource::MarkUploadingDoneThisFrame()
	{
		std::atomic_thread_fence(std::memory_order_release);
		m_SubmitFrame = GetFrameCountContext().GetCurrentFrameID();
	}
	
}


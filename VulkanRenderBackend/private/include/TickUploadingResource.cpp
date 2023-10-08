#include "private/include/pch.h"
#include "TickUploadingResource.h"
#include "CVulkanApplication.h"

namespace graphics_backend
{
	BaseTickingUpdateResource::BaseTickingUpdateResource(CVulkanApplication& app) : BaseApplicationSubobject(app)
	{
	}
	void BaseTickingUpdateResource::SetOwningUpdator(ITickingResourceUpdator* owningUpdator)
	{
		m_OwningUpdator = owningUpdator;
	}
	ITickingResourceUpdator* BaseTickingUpdateResource::GetOwningUpdator() const
	{
		return m_OwningUpdator;
	}
	void BaseTickingUpdateResource::ResetResource()
	{
		b_Dirty = false;
		m_OwningUpdator = nullptr;
		m_SubmitFrame = INVALID_FRAMEID;
	}
	bool BaseTickingUpdateResource::IsValid() const
	{
		return m_OwningUpdator != nullptr;
	}
	bool BaseTickingUpdateResource::IsDirty() const
	{
		return b_Dirty;
	}
	bool BaseTickingUpdateResource::UploadingDone() const
	{
		if (m_SubmitFrame == INVALID_FRAMEID)
			return false;
		auto& frameContext = GetFrameCountContext();
		if (!frameContext.AnyFrameFinished())
			return false;
		return frameContext.GetReleasedFrameID() >= m_SubmitFrame;
	}

	void BaseTickingUpdateResource::MarkDirtyThisFrame()
	{
		if (!b_Dirty)
		{
			b_Dirty = true;
			m_OwningUpdator->EnqueueTickingResource(this);
		}
	}
	void BaseTickingUpdateResource::MarkUploadingDoneThisFrame()
	{
		std::atomic_thread_fence(std::memory_order_release);
		b_Dirty = false;
		m_SubmitFrame = GetFrameCountContext().GetCurrentFrameID();
	}
}

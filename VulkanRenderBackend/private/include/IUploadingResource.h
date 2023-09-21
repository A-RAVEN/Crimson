#pragma once
#include "RenderBackendSettings.h"
#include "VulkanApplicationSubobjectBase.h"
#include "TaskScheduling.h"

namespace graphics_backend
{
	class BaseUploadingResource : public BaseApplicationSubobject
	{
	public:
		BaseUploadingResource(CVulkanApplication& owner);
		void Upload();
		void UploadAsync(UploadingResourceType resourceType);
		bool UploadingDone() const;
	protected:
		virtual void DoUpload() = 0;
		void MarkUploadingDoneThisFrame();
	private:
		FrameType m_SubmitFrame = INVALID_FRAMEID;
	};
}
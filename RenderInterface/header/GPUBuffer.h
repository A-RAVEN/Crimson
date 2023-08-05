#pragma once
#include <RenderInterface/header/Common.h>

namespace graphics_backend
{
	class GPUBuffer
	{
	public:
		virtual void ScheduleBufferData(uint64_t bufferOffset, uint64_t dataSize, void* pData) = 0;
		virtual void DoUpload() = 0;
		virtual bool Submitting() const = 0;
	};
}
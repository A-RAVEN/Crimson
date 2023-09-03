#pragma once
#include <RenderInterface/header/Common.h>

class GPUBuffer
{
public:
	virtual void ScheduleBufferData(uint64_t bufferOffset, uint64_t dataSize, void* pData) = 0;
	virtual void UploadAsync() = 0;
	virtual bool UploadingDone() const = 0;
};

#pragma once
#include <stdint.h>
#include <vector>
#include "Common.h"

class CGPUPrimitiveResource;
class GPUBuffer;

class CInlineCommandList
{
public:
	virtual void BindVertexBuffers(std::vector<GPUBuffer const*> pGPUBuffers, std::vector<uint32_t> offsets) = 0;
	virtual void BindIndexBuffers(EIndexBufferType indexBufferType, GPUBuffer const* pGPUBuffer, uint32_t offset = 0) = 0;
	virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1) = 0;
	virtual void Draw(uint32_t vertexCount, uint32_t instanceCount = 1) = 0;
};


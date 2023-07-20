#pragma once
#include <stdint.h>

class CGPUPrimitiveResource;

class CInlineCommandList
{
public:
	void DrawMesh(CGPUPrimitiveResource const& primitive);
	void Dispatch(uint32_t x, uint32_t y, uint32_t z);
};


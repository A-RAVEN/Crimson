#pragma once
#include <string>
#include <mathfu/vector.h>

class WindowHandle
{
public:
	virtual std::string GetName() const = 0;
	virtual mathfu::Vector<uint32_t, 2> const& GetSize() = 0;
	virtual GPUTextureDescriptor const& GetBackbufferDescriptor() const = 0;
};
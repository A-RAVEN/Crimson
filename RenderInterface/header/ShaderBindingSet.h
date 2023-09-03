#pragma once
#include <RenderInterface/header/Common.h>

class ShaderConstantSet
{
public:
	template<typename T>
	void SetValue(std::string const& name, T const& value)
	{
		SetValue(name, (void*)&value);
	}
	virtual void SetValue(std::string const& name, void* pValue) = 0;
	virtual void UploadAsync() = 0;
	virtual bool UploadingDone() const = 0;
};

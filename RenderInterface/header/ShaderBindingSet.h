#pragma once
#include <RenderInterface/header/Common.h>

class ShaderBindingSet
{
public:
	template<typename T>
	void SetArithmeticValue(std::string const& name, T const& value)
	{
		SetArithmeticValue(name, (void*)&value);
	}
	virtual void SetArithmeticValue(std::string const& name, void* pValue) = 0;
	virtual void DoUpload() = 0;
	virtual bool UploadingDone() const = 0;
};

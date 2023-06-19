#pragma once
#include <string>
#include "Common.h"

//#define DECLARE_API_FUNCTION(APIName, FunctionName, ReturnType, ...)\
//	extern "C"\
//	{\
//		APIName ReturnType FunctionName(__VA_ARGS__);\
//		typedef ReturnType (*FPT_##FunctionName)(__VA_ARGS__);\
//	}

namespace graphics_backend
{
	class CGPUTextureResource
	{
	public:
		virtual CTextureInfo GetTextureInfo() const = 0;
	};

	class CRenderBackend
	{
	public:
		RENDERBACKEND_API virtual void Initialize(std::string const& appName, std::string const& engineName) = 0;
		RENDERBACKEND_API virtual void Release() = 0;
		virtual void NewWindow(uint32_t width, uint32_t height, std::string const& windowName) = 0;
	};



	extern "C"
	{
		RENDERBACKEND_API CRenderBackend* NewRenderBackend();
		RENDERBACKEND_API void DeleteRenderBackend(CRenderBackend* removingBackend);
	}

	void LoadRenderBackend();
	void UnloadRenderBackend();
}




#pragma once
#include <string>

namespace graphics_backend
{
	#ifdef VULKANRENDERBACKEND_EXPORTS
	#define VULKANRENDERBACKEND_API __declspec(dllexport)
	#else
	#define VULKANRENDERBACKEND_API __declspec(dllimport)
	#endif

	class CRenderBackend
	{
	public:
		VULKANRENDERBACKEND_API virtual void Initialize(std::string const& appName, std::string const& engineName) = 0;
		VULKANRENDERBACKEND_API virtual void Release() = 0;
	};


}

extern "C"
{
	VULKANRENDERBACKEND_API graphics_backend::CRenderBackend* NewBackend();
	VULKANRENDERBACKEND_API void DeleteBackend(graphics_backend::CRenderBackend* removingBackend);
}
#pragma once
#include <string>

namespace graphics_backend
{
	#ifdef VULKANRENDERBACKEND_EXPORTS
	#define VULKANRENDERBACKEND_API __declspec(dllexport)
	#else
	#define VULKANRENDERBACKEND_API
	#endif

	class CRenderBackend
	{
	public:
		virtual void Initialize(std::string const& appName, std::string const& engineName) = 0;
		virtual void Release() = 0;
	};

	extern "C"
	{
		VULKANRENDERBACKEND_API CRenderBackend* NewBackend();
		VULKANRENDERBACKEND_API void DeleteBackend(graphics_backend::CRenderBackend* removingBackend);
	}
}
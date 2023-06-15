#pragma once
#include <header/MRenderBackend.h>
#include <private/include/CVulkanApplication.h>
#include <functional>
namespace graphics_backend
{


	class CRenderBackend_Vulkan : public CRenderBackend
	{
	public:
		void Initialize(std::string const& appName, std::string const& engineName) override;
		void Release() override;
	private:
		CVulkanApplication m_Application;
	};
}
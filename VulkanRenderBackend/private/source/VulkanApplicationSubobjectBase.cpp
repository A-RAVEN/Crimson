#include <private/include/pch.h>
#include <private/include/CVulkanApplication.h>
#include <private/include/VulkanApplicationSubobjectBase.h>

namespace graphics_backend
{
	ApplicationSubobjectBase::~ApplicationSubobjectBase()
	{
		Release();
	}

	void ApplicationSubobjectBase::Initialize(CVulkanApplication const* owningApplication)
	{
		m_OwningApplication = owningApplication;
		assert(m_OwningApplication != nullptr);
		Initialize_Internal(owningApplication);
	}

	void ApplicationSubobjectBase::Release()
	{
		Release_Internal();
		m_OwningApplication = nullptr;
	}
}

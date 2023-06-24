#include <private/include/pch.h>
#include <private/include/CVulkanApplication.h>
#include <private/include/VulkanApplicationSubobjectBase.h>

namespace graphics_backend
{
	ApplicationSubobjectBase::~ApplicationSubobjectBase()
	{
		m_OwningApplication = nullptr;
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
	CVulkanApplication const* ApplicationSubobjectBase::GetVulkanApplication() const
	{
		return m_OwningApplication;
	}
	vk::Instance const& ApplicationSubobjectBase::GetInstance() const
	{
		return m_OwningApplication->GetInstance();
	}
	vk::Device const& ApplicationSubobjectBase::GetDevice() const
	{
		return m_OwningApplication->GetDevice();
	}
	vk::PhysicalDevice const& ApplicationSubobjectBase::GetPhysicalDevice() const
	{
		return m_OwningApplication->GetPhysicalDevice();
	}
	CVulkanThreadContext& ApplicationSubobjectBase::GetThreadContext(uint32_t threadIndex)
	{
		return m_OwningApplication->GetThreadContext(threadIndex);
	}
}

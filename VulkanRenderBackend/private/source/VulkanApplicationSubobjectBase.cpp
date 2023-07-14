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

	CFrameCountContext const& ApplicationSubobjectBase::GetFrameCountContext() const
	{
		return m_OwningApplication->GetSubmitCounterContext();
	}

	vk::Instance const& ApplicationSubobjectBase::GetInstance() const
	{
		if (m_OwningApplication == nullptr)
			return nullptr;
		return m_OwningApplication->GetInstance();
	}
	vk::Device const& ApplicationSubobjectBase::GetDevice() const
	{
		if (m_OwningApplication == nullptr)
			return nullptr;
		return m_OwningApplication->GetDevice();
	}
	vk::PhysicalDevice const& ApplicationSubobjectBase::GetPhysicalDevice() const
	{
		if (m_OwningApplication == nullptr)
			return nullptr;
		return m_OwningApplication->GetPhysicalDevice();
	}
	CVulkanThreadContext* ApplicationSubobjectBase::GetThreadContext(uint32_t threadIndex)
	{
		if (m_OwningApplication == nullptr)
			return nullptr;
		return m_OwningApplication->GetThreadContext(threadIndex);
	}
}
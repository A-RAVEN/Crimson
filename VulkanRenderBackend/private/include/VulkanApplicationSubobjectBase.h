#pragma once
namespace graphics_backend
{
#pragma region Forward Declaration
	class CVulkanApplication;
#pragma endregion

	class ApplicationSubobjectBase
	{
	public:
		virtual void Initialize(CVulkanApplication const* owningApplication);
		virtual void Release();
		CVulkanApplication const* GetVulkanApplication() const { return m_OwningApplication; }
		vk::Instance const& GetInstance() const { return m_OwningApplication->GetInstance(); }
		vk::Device const& GetDevice() const { return m_OwningApplication->GetDevice(); }
		virtual ~ApplicationSubobjectBase();
	protected:
		virtual void Initialize_Internal(CVulkanApplication const* owningApplication) = 0;
		virtual void Release_Internal() = 0;
		CVulkanApplication const* m_OwningApplication = nullptr;
	};
}
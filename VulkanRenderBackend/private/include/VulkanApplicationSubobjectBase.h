#pragma once
namespace graphics_backend
{
#pragma region Forward Declaration
	class CVulkanApplication;
#pragma endregion

	class ApplicationSubobjectBase
	{
	public:
		virtual ~ApplicationSubobjectBase();
		virtual void Initialize(CVulkanApplication const* owningApplication);
		virtual void Release();
	protected:
		virtual void Initialize_Internal(CVulkanApplication const* owningApplication) = 0;
		virtual void Release_Internal() = 0;
		CVulkanApplication const* m_OwningApplication = nullptr;
	};
}
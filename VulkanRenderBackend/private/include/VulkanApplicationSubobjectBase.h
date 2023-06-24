#pragma once
namespace graphics_backend
{
#pragma region Forward Declaration
	class CVulkanApplication;
	class CVulkanThreadContext;
#pragma endregion

	class ApplicationSubobjectBase
	{
	public:
		virtual void Initialize(CVulkanApplication const* owningApplication);
		virtual void Release();
		CVulkanApplication const* GetVulkanApplication() const;
		vk::Instance const& GetInstance() const;
		vk::Device const& GetDevice() const;
		vk::PhysicalDevice const& GetPhysicalDevice() const;
		CVulkanThreadContext& GetThreadContext(uint32_t threadIndex);
		virtual ~ApplicationSubobjectBase();
	protected:
		virtual void Initialize_Internal(CVulkanApplication const* owningApplication) = 0;
		virtual void Release_Internal() = 0;
		CVulkanApplication const* m_OwningApplication = nullptr;
	};
}
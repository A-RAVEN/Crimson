#pragma once

#if _WIN32 || _WIN64
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define CURRENT_VULKAN_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#if _WIN64
#define VKHANDLE_CAST reinterpret_cast
#else
#define VKHANDLE_CAST static_cast
#endif
#endif

#define CURRENT_VULKAN_VERSION VK_API_VERSION_1_0

#include <vulkan/vulkan.h>

#define VULKAN_ALLOCATOR_POINTER nullptr
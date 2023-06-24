#pragma once

#if defined (_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR 1
#elif defined(__linux__)
#define VK_USE_PLATFORM_XCB_KHR 1
#elif defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR 1
#endif

#ifndef VULKAN_HPP_TYPESAFE_CONVERSION 
#define VULKAN_HPP_TYPESAFE_CONVERSION 1
#endif
#include "vulkan/vulkan.hpp"

#define VULKAN_API_VERSION_IN_USE VK_API_VERSION_1_3